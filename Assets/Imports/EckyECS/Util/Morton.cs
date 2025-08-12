using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.CompilerServices;
using Unity.Burst;
using Unity.Burst.Intrinsics;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using Unity.Jobs;
using Unity.Jobs.LowLevel.Unsafe;
using static Unity.Burst.Intrinsics.X86.Avx;
using static Unity.Burst.Intrinsics.X86.Avx2;

/** Converts a world position into a morton code
 * referencing a fixed offset to make the space positive only
 * Interlaces a casted short into an int, so max range is 2^16 ~= 65k
 */
[BurstCompile(OptimizeFor = OptimizeFor.Performance)]
public unsafe struct MortonSIMDJob : IJobParallelFor
{
    [ReadOnly]
    public NativeArray<TransformComponent> Components;

    [NativeDisableParallelForRestriction]
    [WriteOnly]
    public NativeArray<uint> MortonCodes;

    public int WorkerCount;

    private int SIMDSize;
    private int SIMDCount;
    private int StandardCount;
    private int CountPerThread;
    private int LeftOverCount;

    public int Iteration;

    [NativeDisableUnsafePtrRestriction]
    private v256* MortonPtr;
    [NativeDisableUnsafePtrRestriction]
    [ReadOnly]
    private v256* ComponentsPtr;

    public void Execute(int i)
    {
        // equally distribute load
        CountPerThread = Components.Length / WorkerCount;       // 50k / 20 => 2500
        SIMDSize = 8;
        float SIMDCountPerThread = CountPerThread / SIMDSize;       // 2.5k / 8 => 312.5
        SIMDCount = (int)SIMDCountPerThread;                        // 312 
        StandardCount = CountPerThread - SIMDCount * SIMDSize;      // 4
        LeftOverCount = Components.Length - CountPerThread * WorkerCount;
        MortonPtr = (v256*)MortonCodes.GetUnsafePtr();
        ComponentsPtr = (v256*)Components.GetUnsafeReadOnlyPtr();

        SIMDCall(i);
        StandardCall(i);
        LeftOverCall(i);

        //MortonCodes[0] = 1;
        //MortonCodes[1] = 2;
        //MortonCodes[2] = 4;
        //MortonCodes[3] = 5;
        //MortonCodes[4] = 19;
        //MortonCodes[5] = 24;
        //MortonCodes[6] = 25;
        //MortonCodes[7] = 30;
    }

    /** SIMD write a chunk of memory, starting at a chunked offset according to thread index */
    private readonly void SIMDCall(int i)
    {
        // a v256 contains 8 floats, but we access 3 per iteration to get 8 whole transform components
        int VectorsPerIteration = 3;
        int Offset = i * SIMDCount;

        // we wanna merge three v256 into two (x and z comps), so shuffle them around that they fit
        // forward and backward into it. Then morton code and vadd them back together (z should be reversed to keep order)
        // maps x0 y0 z0 x1 y1 z1 x2 y2
        // into x0 x1 x2 __ __ __ z1 z0
        v256 Shuffle0 = new v256(0, 3, 6, 1, 4, 7, 5, 2);

        // since both x and z would need to be in this vector, we have to make two
        // maps z2 x3 y3 z3 x4 y4 z4 x5
        // into __ __ __ x3 x4 x5 __ __ and
        //      __ __ __ z4 z3 z2 __ __ 
        v256 Shuffle11 = new v256(0, 3, 6, 1, 4, 7, 2, 5);
        v256 Shuffle12 = new v256(1, 4, 7, 6, 3, 0, 2, 5);

        // maps y5 z5 x6 y6 z6 x7 y7 z7
        // into z7 z6 z5 __ __ __ x6 x7
        v256 Shuffle2 = new v256(7, 4, 1, 0, 3, 6, 2, 5);
        v256 ReverseShuffle = new v256(7, 6, 5, 4, 3, 2, 1, 0);

        const uint y = 0xFFFFFFFF;
        v256 Mask0 = new v256(y, y, y, 0, 0, 0, 0, 0);
        v256 Mask1 = new v256(0, 0, 0, y, y, y, 0, 0);
        v256 Mask2 = new v256(0, 0, 0, 0, 0, 0, y, y);
        for (int j = 0; j < SIMDCount; j++)
        {
            int CompsOffset = (Offset + j) * VectorsPerIteration;
            int MortonOffset = Offset + j;

            // since the components are interlaced and we only want x and z location (y is ignored in morton)
            // we need to offset/read from different pointers
            // comps: x, y, z, x, y, z, x, y, z, x, y, z, x, y, z, x, y, z, x, y, z, x, y, z
            // x ind: 0        1        2        3        4        5        6        7
            // z ind:       0        1        2        3        4        5        6        7
            // v pos: 00,01,02,03,04,05,06,07,10,11,12,13,14,15,16,17,20,21,22,23,24,25,26,27
            // (also reverse)

            v256 Comps0 = mm256_cvtps_epi32(mm256_loadu_ps(ComponentsPtr + CompsOffset + 0));
            v256 Comps1 = mm256_cvtps_epi32(mm256_loadu_ps(ComponentsPtr + CompsOffset + 1));
            v256 Comps2 = mm256_cvtps_epi32(mm256_loadu_ps(ComponentsPtr + CompsOffset + 2));

            v256 sComps0 = mm256_permutevar8x32_epi32(Comps0, Shuffle0);
            v256 sComps11 = mm256_permutevar8x32_epi32(Comps1, Shuffle11);
            v256 sComps12 = mm256_permutevar8x32_epi32(Comps1, Shuffle12);
            v256 sComps2 = mm256_permutevar8x32_epi32(Comps2, Shuffle2);

            v256 xComp =
                mm256_or_si256(mm256_and_si256(sComps0, Mask0),
                mm256_or_si256(mm256_and_si256(sComps11, Mask1), mm256_and_si256(sComps2, Mask2)));
            v256 zComp =
                mm256_or_si256(mm256_and_si256(sComps0, Mask2),
                mm256_or_si256(mm256_and_si256(sComps12, Mask1), mm256_and_si256(sComps2, Mask0)));
            zComp = mm256_permutevar8x32_epi32(zComp, ReverseShuffle);

            mm256_storeu_si256(MortonPtr + MortonOffset, Morton.InterlaceSIMD(xComp, zComp));
        }
    }

    /** Regularly compute morton codes, but write them after the chunked blocks from before (easier offset calc)*/
    private void StandardCall(int i)
    {
        for (int j = 0; j < StandardCount; j++)
        {
            int Offset = WorkerCount * SIMDCount * SIMDSize + i * StandardCount + j;
            MortonCodes[Offset] = Morton.Interlace(Components[Offset].PosX, Components[Offset].PosZ);
        }
    }

    private void LeftOverCall(int i)
    {
        if (i < WorkerCount - 1)
            return;

        for (int j = 0; j < LeftOverCount; j++)
        {
            int Offset = WorkerCount * CountPerThread + j;
            MortonCodes[Offset] = Morton.Interlace(Components[Offset].PosX, Components[Offset].PosZ);
        }
    }
}

[BurstCompile(OptimizeFor = OptimizeFor.Performance)]
public unsafe struct MortonJob : IJobParallelFor
{
    [ReadOnly]
    public NativeArray<TransformComponent> Components;

    [WriteOnly]
    public NativeArray<uint> MortonCodes;

    public int WorkerCount;

    public void Execute(int i)
    {
        MortonCodes[i] = Morton.Interlace(Components[i].PosX, Components[i].PosZ);
    }

}

public class Morton {

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static uint Interlace(UnityEngine.Vector3 V)
    {
        return Interlace((int)V.x, (int)V.z);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static uint Interlace(float A, float B)
    {
        return Interlace((int)A, (int)B);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static uint Interlace(int A, int B)
    {
        return Interlace((uint)(A + CenterOffset)) | (Interlace((uint)(B + CenterOffset)) << 1);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    private static uint Interlace(uint N)
    {
        // https://snorrwe.onrender.com/posts/morton-table/
        N = (N ^ (N << 8)) & 0x00ff00ff; 
        N = (N ^ (N << 4)) & 0x0f0f0f0f; 
        N = (N ^ (N << 2)) & 0x33333333; 
        return (N ^ (N << 1)) & 0x55555555; 
    }

    public static v256 InterlaceSIMD(v256 A, v256 B)
    {
        var A1 = InterlaceSIMD(mm256_add_epi32(A, new(CenterOffset)));
        var B1 = InterlaceSIMD(mm256_add_epi32(B, new(CenterOffset)));
        var B2 = mm256_sllv_epi32(B1, new(1));
        return mm256_or_si256(A1, B2);
    }

    public static v256 InterlaceSIMD(v256 N)
    {
        N = mm256_and_ps(mm256_xor_ps(N, mm256_sllv_epi32(N, new(08))), new(0x00ff00ff));
        N = mm256_and_ps(mm256_xor_ps(N, mm256_sllv_epi32(N, new(04))), new(0x0f0f0f0f));
        N = mm256_and_ps(mm256_xor_ps(N, mm256_sllv_epi32(N, new(02))), new(0x33333333));
        return mm256_and_ps(mm256_xor_ps(N, mm256_sllv_epi32(N, new(01))), new(0x55555555)); ;
    }

    public static void DeInterlace(uint M, out int A, out int B)
    {
        uint Au = DeInterlace(M, false);
        uint Bu = DeInterlace(M, true);
        A = (int)(Au - CenterOffset);
        B = (int)(Bu - CenterOffset);
    }

    private static uint DeInterlace(uint N, bool bIsOther)
    {
        N = N >> (bIsOther ? 1 : 0); 
        uint A = ((N & 0x44444444) >> 1) | (N & 0x11111111);
        uint B = ((A & 0x30303030) >> 2) | (A & 0x03030303);
        uint C = ((B & 0x0F000F00) >> 4) | (B & 0x000F000F);
        uint D = ((C & 0x00FF0000) >> 8) | (C & 0x000000FF);
        return D;
    }

    public static string GetBitString(uint Value)
    {
        byte[] Bytes = BitConverter.GetBytes(Value);
        return string.Join(" ",
            Bytes.Reverse().Select(x => Convert.ToString(x, 2).PadLeft(8, '0')));
    }

    public static string GetBitString(v256 v)
    {
        byte[] Bytes = new byte[32]
        {
            v.Byte0, v.Byte1, v.Byte2, v.Byte3, v.Byte4,
            v.Byte5, v.Byte6, v.Byte7, v.Byte8, v.Byte9,
            v.Byte10, v.Byte11, v.Byte12, v.Byte13, v.Byte14,
            v.Byte15, v.Byte16, v.Byte17, v.Byte18, v.Byte19,
            v.Byte20, v.Byte21, v.Byte22, v.Byte23, v.Byte24,
            v.Byte25, v.Byte26, v.Byte27, v.Byte28, v.Byte29,
            v.Byte30, v.Byte31
        };
        return string.Join(" ",
            Bytes.Reverse().Select(x => Convert.ToString(x, 2).PadLeft(8, '0')));
    }

    private readonly static int CenterOffset = short.MaxValue;
}
