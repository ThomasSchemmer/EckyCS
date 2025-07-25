
Shader "Custom/WaterShader"
{
    Properties
    {
        [Header(Waves)][Space]
        _WaveDirection("Direction", Vector) = (0,0,0,0)
        _WaveLength("Length", Vector) = (0,0,0,0)
        _WaveSteepness("Steepness", Vector) = (0,0,0,0)
        _WaveSpeed("Speed", Vector) = (0,0,0,0)
        _QuantizeAmountWave("Quantize", Float) = 1

        [Header(Color)][Space]
        _TopColor("TopColor", Color) = (0,0,0,1)
        _BottomColor("BottomColor", Color) = (0,0,0,1)
        _MaxDepth("MaxDepth", Float) = 20
        _QuantizeAmountDepth("Quantize", Float) = 5
        _SeeThrough("See Through", Range(0, 5)) = .5

        [Header(Caustic)][Space]
        _CausticTexture ("Texture", 2D) = "white" {}
        _CausticHighlightTexture ("Highlight", 2D) = "white" {}
        _CausticScale("Scale", Float) = 1
        _CausticDistortionStrength("Distortion Strength", Float) = 1
        _CausticDistortionScale("Distortion Scale", Float) = 1
        _CausticMin("Min", Float) = 0
        _CausticMax("Max", Float) = 1
        _CausticPixel("Pixel", Float) = 1
        _CausticColor("Color", Color) = (0,0,0,1)
        _CausticHighlightColor("Highlight Color", Color) = (0,0,0,1)
        _CausticVector("Vector", Vector) = (0,0,0,1)
        
        [Header(Foam)][Space]
        _FoamColor("Color", Color) = (0,0,0,1)
        _FoamHeightCutoff("Height Cutoff", Float) = 0.2
        _FoamTopAlphaCutoff("Top Alpha Cutoff", Float) = 0.2
        _FoamDepthAlphaCutoff("Depth Alpha Cutoff", Float) = 0.2
        _FoamDepthCutoff("Depth Cutoff", Float) = 0.2
        _FoamNormalCutoff("Normal Cutoff", Float) = 0.2
        _FoamScale("Scale", Float) = 1
    }
    

    HLSLINCLUDE
    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
    #include "Util/GpuPrinter.cginc"
    #include "Util/Cubic.cginc"
    #include "Util/WorldPos.cginc" 

    CBUFFER_START(UnityPerMaterial)
        //waves
        float4 _WaveDirection;
        float4 _WaveLength;
        float4 _WaveSteepness;
        float4 _WaveSpeed;
        float _QuantizeAmountWave;

        // depth color
        float4 _TopColor;
        float4 _BottomColor;
        float _MaxDepth;
        float _QuantizeAmountDepth;
        float _SeeThrough;

        // caustic
        sampler2D _CausticTexture;
        sampler2D _CausticHighlightTexture;
        float4 _CausticColor;
        float4 _CausticHighlightColor;
        // contains water direction in xy
        float4 _CausticVector;
        float _CausticScale;
        float _CausticMin;
        float _CausticMax;
        float _CausticPixel;
        float _CausticDistortionStrength;
        float _CausticDistortionScale;

        // Foam
        float4 _FoamColor;
        float _FoamHeightCutoff;
        float _FoamDepthCutoff;
        float _FoamNormalCutoff;
        float _FoamScale;
        float _FoamTopAlphaCutoff;
        float _FoamDepthAlphaCutoff;

        // utils
        sampler2D _CameraDepthTexture;
        sampler2D _CameraNormalsTexture;
    CBUFFER_END
    ENDHLSL
        
    SubShader
    {
        Tags { "RenderType"="Transparent" "LightMode" = "UniversalForward"  }
        //Tags { "RenderType"="Opaque" "LightMode" = "UniversalForward"  }    //UniversalGBuffer
        LOD 100
        Cull Back
        ZWrite On
        ZTest LEqual
	    Blend SrcAlpha OneMinusSrcAlpha 

        
        Pass
        {
            HLSLPROGRAM
            
            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Lighting.hlsl" // shadows and ambient light

            #pragma vertex vert
            #pragma fragment frag
            #pragma multi_compile _ _MAIN_LIGHT_SHADOWS
            #pragma multi_compile _ _MAIN_LIGHT_SHADOWS_CASCADE
            #pragma multi_compile _ _SHADOWS_SOFT
                        

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
                float3 normal : NORMAL;
            };

            struct v2f
            {
                float4 clip : SV_POSITION;
                float2 uv : TEXCOORD0;
                float3 world : TEXCOORD1;
                float2 screen : TEXCOORD2;
                float3 object : TEXCOORD3;
            };

            // https://ameye.dev
            float3 GerstnerWave(float3 position, float steepness, float wavelength, float speed, float direction, inout float3 tangent, inout float3 binormal)
            {
                direction = direction * 2 - 1;
                float2 d = normalize(float2(cos(PI * direction), sin(PI * direction)));
                float k = 2 * 3.14 / wavelength;
                float f = k * (dot(d, position.xz) - speed * _Time.y);
                float a = steepness / k;

                tangent += float3(
                    1 - d.x * d.x * (steepness * sin(f)),
                    d.x * (steepness * cos(f)),
                    -d.x * d.y * (steepness * sin(f))
                );

                binormal += float3(
                    -d.x * d.y * (steepness * sin(f)),
                    d.y * (steepness * cos(f)),
                    1 - d.y * d.y * (steepness * sin(f))
                );

                return float3(
                    d.x * (a * cos(f)),
                    a * sin(f),
                    d.y * (a * cos(f))
                );
            }


            void GetWave(float3 ObjectPos, out float3 Position, out float3 Normal){
            
			    float3 tangent = float3(0, 0, 0);
			    float3 binormal = float3(0, 0, 0);
			    Position = ObjectPos;
			    Position += GerstnerWave(ObjectPos, _WaveSteepness.x, _WaveLength.x, _WaveSpeed.x, _WaveDirection.x, tangent, binormal);
			    Position += GerstnerWave(ObjectPos, _WaveSteepness.y, _WaveLength.y, _WaveSpeed.y, _WaveDirection.y, tangent, binormal);
			    Position += GerstnerWave(ObjectPos, _WaveSteepness.z, _WaveLength.z, _WaveSpeed.z, _WaveDirection.z, tangent, binormal);
			    Position += GerstnerWave(ObjectPos, _WaveSteepness.w, _WaveLength.w, _WaveSpeed.w, _WaveDirection.w, tangent, binormal);
                
                Normal = normalize(cross(tangent, binormal));
            }

            v2f vert (appdata v)
            {
                v2f o;
                
                float3 Position, Normal;
                GetWave(v.vertex.xyz, Position, Normal);
                VertexPositionInputs vertexInput = GetVertexPositionInputs(Position);
                
                o.clip = vertexInput.positionCS;
                o.world = vertexInput.positionWS;
                o.screen = ComputeScreenPos(o.clip).xy;
                o.object = v.vertex.xyz;
                o.uv = v.uv;
                return o;
            }

            float4 GetCausticColor(float3 WorldPos, float d){
                float Limit = clamp(d, _CausticMin, _CausticMax);
                float2 DistortionUV = ((WorldPos.xz + _Time.y * _CausticVector.xy) / _CausticDistortionScale);
                float2 Distortion = cubicNoise(float3(DistortionUV.x, 0, DistortionUV.y)) * _CausticDistortionStrength;
                float2 UV = (WorldPos.xz / _CausticScale) + Distortion;
                UV = (int2)(UV * _CausticPixel) / _CausticPixel;

                float4 Color = tex2D(_CausticTexture, UV).a * Limit * _CausticColor;
                float4 Highlight = tex2D(_CausticHighlightTexture, UV).a * Limit * _CausticHighlightColor;
                return Color + Highlight;
            }
            
            float GetWorldDepth(v2f i, float3 WorldDepthPos){
                float d = distance(WorldDepthPos, i.world);
                // bring into 0..1
                d = 1 - clamp(d, 0, _MaxDepth) / _MaxDepth;
                // quantize and make lowest band also slightly colored
                d = ((int)(ceil(d * _QuantizeAmountDepth))) / _QuantizeAmountDepth;
                return d;
            }

            float4 GetLight(float3 WorldNormal, float4 Color){
            
                Light MainLight = GetMainLight();
                half nl = max(0, dot(WorldNormal, MainLight.direction.xyz));
                nl = ((int)(ceil(nl * _QuantizeAmountWave))) / _QuantizeAmountWave;
                return Color * nl;
            }

            float GetShadow(float3 WorldPos){
            
                VertexPositionInputs vertexInput = (VertexPositionInputs)0;
                vertexInput.positionWS = WorldPos;
                float4 shadowCoord = GetShadowCoord(vertexInput);
                half shadowAttenutation = MainLightRealtimeShadow(shadowCoord);
                return shadowAttenutation;
            }

            float4 GetFoamColor(float3 WorldPos, float3 DepthPos, float3 WorldNormal){
                float d = distance(DepthPos, WorldPos);
                d = 1 - clamp(d / _MaxDepth, 0, 1);
                float A = 0;
                float NormalA = pow(dot(WorldNormal, float3(0, 1, 0)), 3);
                NormalA = smoothstep(_FoamNormalCutoff, 1, NormalA);
                float HeightA = smoothstep(_FoamHeightCutoff, _FoamHeightCutoff + 0.2, WorldPos.y);
                float TopA = min(NormalA, HeightA);
                float DepthA = smoothstep(_FoamDepthCutoff, 1, d);

                float Noise = cubicNoise(WorldPos * _FoamScale + _Time.y);
                TopA *= smoothstep(_FoamTopAlphaCutoff, 1, Noise);
                DepthA *= smoothstep(_FoamDepthAlphaCutoff, 1, Noise);

                A = TopA + DepthA;
                A = saturate(A);
                A = 1 - (1 - A) * (1 - A);
                return float4(_FoamColor.xyz * A, 1);
            }

            float4 frag (v2f i) : SV_TARGET
            {
                float3 TempPosition, TempNormal;
                GetWave(i.object, TempPosition, TempNormal);
                float3 WorldNormal = float3(TempNormal.x, -TempNormal.y, TempNormal.z);
                
                float Depth = tex2D(_CameraDepthTexture, i.screen).x;
                float3 WorldDepthPos = ComputeWorldSpacePosition(i.screen, Depth, UNITY_MATRIX_I_VP);
                float WorldDepth = GetWorldDepth(i, WorldDepthPos);
                
                float4 BaseColor = lerp(_BottomColor, _TopColor, WorldDepth);
                BaseColor = GetLight(WorldNormal, BaseColor);

                float4 CausticColor = GetCausticColor(i.world, WorldDepth);
                float4 FoamColor = GetFoamColor(i.world, WorldDepthPos, WorldNormal);
                float4 Color = saturate(BaseColor + CausticColor + FoamColor);

                //float shadowAttenutation = GetShadow(i.world);
                //float4 MixedColor = lerp(float4(0.01, 0.01, 0.03, 1), Color, shadowAttenutation);
                return float4(Color.xyz, _SeeThrough - WorldDepth);
            }
            ENDHLSL
        }
        
    }
}
