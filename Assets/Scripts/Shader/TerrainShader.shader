Shader "Custom/Terrain"
{
    Properties
    {
        _GrassColor("Grass", Color) = (0,0,0,0)
        _RockColor("Rock", Color) = (0,0,0,0)
        _Quantize("Quantize", Float) = 1
        _GrassThreshold("Grass Threshold", Range(0, 1)) = 1

    }

    
    HLSLINCLUDE

    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Lighting.hlsl" // shadows and ambient light

    CBUFFER_START(UnityPerMaterial)
        float4 _GrassColor;
        float4 _RockColor;
        float _Quantize;
        float _GrassThreshold;

    CBUFFER_END

    ENDHLSL

    SubShader
    {
        Tags { "RenderType"="Opaque" "LightMode" = "UniversalForward"  }
        LOD 100

        Pass
        {
            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #pragma multi_compile _ _MAIN_LIGHT_SHADOWS
            #pragma multi_compile _ _MAIN_LIGHT_SHADOWS_CASCADE
            #pragma multi_compile _ _SHADOWS_SOFT
            
           

            struct appdata
            {
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float4 vertex : SV_POSITION;
                float3 world : TEXCOORD1;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;
            };
            

            v2f vert (appdata v)
            {
                v2f o;
                VertexPositionInputs vertexInput = GetVertexPositionInputs(v.vertex.xyz);
                VertexNormalInputs normalInputs = GetVertexNormalInputs(v.normal.xyz);
                
                o.vertex = vertexInput.positionCS;
                o.world = vertexInput.positionWS;
                o.normal = v.normal.xyz;//normalInputs.normalWS;
                o.uv = v.uv;
                return o;
            }

            
            float GetShadow(float3 WorldPos){
            
                VertexPositionInputs vertexInput = (VertexPositionInputs)0;
                vertexInput.positionWS = WorldPos;
                float4 shadowCoord = GetShadowCoord(vertexInput);
                half shadowAttenutation = MainLightRealtimeShadow(shadowCoord);
                return shadowAttenutation;
            }

            float4 frag (v2f i) : SV_Target
            {
                Light light = GetMainLight();
                float nl = max(0, dot(i.normal, light.direction.xyz));
                nl = (int)(nl * _Quantize) / _Quantize;

                float3 Normal = abs(normalize(i.normal));
                float GrassFactor = smoothstep(_GrassThreshold, 1, Normal.y);
                float RockFactor = dot(float3(Normal.x, 0, Normal.z), float3(1, 0, 0)) + dot(float3(Normal.x, 0, Normal.z), float3(0, 0, 1));  
                float4 Color = GrassFactor * _GrassColor + RockFactor * _RockColor;
                Color.xyz *= nl;

                float shadowAttenutation = GetShadow(i.world);
                return lerp(float4(0.01, 0.01, 0.03, 1), Color, shadowAttenutation);

            }
            ENDHLSL
        }
    
        // DepthNormalsOnly Pass
        Pass
        {
            Tags { "RenderType"="Opaque"  "LightMode" = "DepthNormalsOnly" }
            
            HLSLPROGRAM
            #pragma vertex DepthNormalsVertex
            #pragma fragment DepthNormalsFragment

            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
            #include "Packages/com.unity.render-pipelines.universal/Shaders/LitInput.hlsl"

            struct Attributes
            {
                float4 positionOS : POSITION;
                float3 normalOS : NORMAL;
            };

            struct Varyings
            {
                float4 positionCS : SV_POSITION;
                float3 normalWS : TEXCOORD0;
            };

            Varyings DepthNormalsVertex(Attributes input)
            {
                Varyings output;
                output.positionCS = TransformObjectToHClip(input.positionOS.xyz);
                output.normalWS = TransformObjectToWorldNormal(input.normalOS);
                return output;
            }

            half4 DepthNormalsFragment(Varyings input) : SV_TARGET
            {
                return float4(input.normalWS * 0.5 + 0.5, 1);
            }
            ENDHLSL
        }
        
    }
    FallBack "Mobile/Diffuse"
}
