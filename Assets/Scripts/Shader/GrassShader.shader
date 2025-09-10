Shader "Custom/Grass"
{
    Properties
    {
        _GrassTex ("Grass", 2D) = "white" {}
        _CopyTex ("CopyTex", 2D) = "white" {}
        _TargetTex ("TargetTex", 2D) = "white" {}
        _Cutoff("Cutoff", Float) = 0
        _Offset("Offset", Vector) = (0,0,0,0)
    }

    
    HLSLINCLUDE

    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Lighting.hlsl" // shadows and ambient light
    #include "Util/GpuPrinter.cginc"
    #include "Util/WorldPos.cginc" 

    #pragma enable_d3d11_debug_symbols

    CBUFFER_START(UnityPerMaterial)
        sampler2D _GrassTex;
        sampler2D _CopyTex;
        sampler2D _TargetTex;
        
        sampler2D _CameraDepthTexture;
        float4 _CamOffset;
        float4 _Offset;
        float _Cutoff;
    CBUFFER_END

    ENDHLSL

    SubShader
    {
        Tags { "RenderType"="Transparent" "LightMode" = "UniversalForward"  }
        LOD 100
	    Blend SrcAlpha OneMinusSrcAlpha 
        ZTest LEqual
        //ZWrite Off

        Pass
        {
            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #pragma target 4.5

            #pragma multi_compile_instancing

            StructuredBuffer<float3> PositionBuffer;
            
            struct appdata
            {
                uint vertexID : SV_VERTEXID;
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;
                UNITY_VERTEX_INPUT_INSTANCE_ID
            };

            struct v2f
            {
                float4 vertex : SV_POSITION;
                float3 normal : NORMAL;
                // contains vertexSP (x, y) and centerSP(z,w)
                float4 screen : TEXCOORD0;
                float3 world : TEXCOORD1;
                // used to interpolate UVs, contains min(x,y) and max(z, w)
                float4 screenDims : TEXCOORD2;
            };

            static float3 Vertices[] = {
                float3(-.5, 0, 0),
                float3(.5, 0, 0),
                float3(-.5, 1, 0),
                float3(.5, 1, 0),
            };

            static int ColorHeight = 256;
            static int ColorWidth = 455;
            static float Size = 2;
            static float2 ColorSize = float2(ColorHeight, ColorWidth);
            static float2 WorldSpaceScreen = float2(37.6, 41.66);

            float2 GetScreenPos(float3 Pos){
                VertexPositionInputs vertexInput = GetVertexPositionInputs(Pos);
                float2 ScreenPos = (vertexInput.positionCS.xy / vertexInput.positionCS.w) * .5 + .5;
                //ScreenPos = (int2)(ScreenPos * _ScreenParams.xy) / _ScreenParams.xy;
                return ScreenPos;
            }

            float4 SnapToPixelCS(float3 Pos){
                // transform to screen pos, snap to pixel, convert back to clip
                VertexPositionInputs vertexInput = GetVertexPositionInputs(Pos.xyz);
                float2 ScreenPos = GetScreenPos(Pos);
                float4 PositionCS = float4(ScreenPos, vertexInput.positionCS.zw);
                PositionCS.xy = (PositionCS.xy * 2 - 1) / vertexInput.positionCS.w;
                return PositionCS;
            }

            float4 GetWorldPos(int VertexIndex, int InstanceID){
                float3 Vert = Vertices[VertexIndex];
                float3 Offset = 
                    Vert.x * _CamRight.xyz +
                    Vert.y * _CamUp.xyz;
                float3 Pos = PositionBuffer[InstanceID] + Offset * Size;
                return float4(Pos, 1);
            }
            
            v2f vert (appdata v, uint InstanceID : SV_INSTANCEID)
            {
                UNITY_SETUP_INSTANCE_ID(v);
                v2f o;

                float3 Pos = GetWorldPos(v.vertexID, InstanceID).xyz;
                VertexPositionInputs VertexInput = GetVertexPositionInputs(Pos);
                VertexPositionInputs BaseVertexInput = GetVertexPositionInputs(PositionBuffer[InstanceID]);
                VertexPositionInputs Vertex0Input = GetVertexPositionInputs(GetWorldPos(0, InstanceID));
                VertexPositionInputs Vertex3Input = GetVertexPositionInputs(GetWorldPos(3, InstanceID));
                
                o.vertex = VertexInput.positionCS;//SnapToPixelCS(Pos); 
                o.normal = -_CamForward.xyz;
                o.world = BaseVertexInput.positionWS;
                o.screen = float4(
                    ComputeScreenPos(VertexInput.positionCS).xy,
                    ComputeScreenPos(BaseVertexInput.positionCS).xy//GetScreenPos(PositionBuffer[InstanceID]).xy
                );
                o.screenDims = float4(
                    ComputeScreenPos(Vertex0Input.positionCS).xy, 
                    ComputeScreenPos(Vertex3Input.positionCS).xy
                );
                
                return o;
            }

            float4 frag (v2f i) : SV_Target
            {
                //float4 ColorA = float4(i.screen.xy, 0, 1);
                //float2 uv = (i.screen.xy - i.screenDims.xy) / (i.screenDims.zw - i.screenDims.xy); //
                //float4 ColorB = float4(uv, 0, 1);
                //return uv.x < .5 ? ColorB : ColorA;

                float2 uv = (i.screen.xy - i.screenDims.xy) / (i.screenDims.zw - i.screenDims.xy); 
                float Depth = tex2D(_CameraDepthTexture, i.screen).x;
                float3 WorldDepthPos = ComputeWorldSpacePosition(i.screen, Depth, UNITY_MATRIX_I_VP);
                float Dis = distance(WorldDepthPos, i.world);
                clip(_Cutoff - Dis);

                float4 GrassColor = tex2D(_GrassTex, uv);
                clip(GrassColor.a - 0.5);

                float2 ScreenUV = (i.screen.zw * ColorSize) / ColorSize;
                ScreenUV = clamp(ScreenUV, 0, 1);
                float4 Color = tex2D(_CopyTex, ScreenUV);
                return float4(Color.xyz, 1);
            }
            ENDHLSL
        }
    
    }
}
