/** 
    Per object shader to store pixelize data into the GBuffers and render regularly
    into a small RT
*/
Shader "Custom/PixelizeShader"
{
    Properties
    {
    _Scale("Scale", Float) = 1
    }
    

    HLSLINCLUDE
    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"

    CBUFFER_START(UnityPerMaterial)
    CBUFFER_END
        float _Scale;

    ENDHLSL
        
    SubShader
    {
        Tags { "RenderType"="Opaque" "LightMode" = "UniversalForward"  }    //UniversalGBuffer
        LOD 100
        Cull Back
        ZWrite On
        ZTest LEqual

        
        Pass
        {
            HLSLPROGRAM
            
            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Lighting.hlsl" // shadows and ambient light
            #pragma vertex vert
            #pragma fragment frag
            
			#pragma exclude_renderers nomrt
            #pragma multi_compile_fwdbase nolightmap nodirlightmap nodynlightmap novertexlight
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
                float2 uv : TEXCOORD0;
                float4 clip : SV_POSITION;
                float3 world : TEXCOORD1;
                float3 normal : NORMAL;
            };

            v2f vert (appdata v)
            {
                v2f o;
                
                VertexPositionInputs vertexInput = GetVertexPositionInputs(v.vertex.xyz);
                o.clip = vertexInput.positionCS;
                o.world = vertexInput.positionWS;
                o.normal = GetVertexNormalInputs(v.normal.xyz).normalWS;
                o.uv = 0;
                return o;
            }

            float4 frag (v2f i, out float Depth : SV_DEPTH) : SV_TARGET
            {
                VertexPositionInputs vertexInput = (VertexPositionInputs)0;
                vertexInput.positionWS = i.world;
                //#if UNITY_REVERSED_Z
                //return 1;
                //#endif
                Light light = GetMainLight();
                float4 shadowCoord = GetShadowCoord(vertexInput);
                half shadowAttenutation = MainLightRealtimeShadow(shadowCoord);
                half nl = max(0, dot(i.normal, light.direction.xyz));
                float4 baseColor = float4(0.3, 0.45, 0.4, 1);
                baseColor.xyz *= nl;

                Depth = ComputeNormalizedDeviceCoordinatesWithZ(i.world, GetWorldToHClipMatrix()).z;
                
                return lerp(float4(0.01, 0.01, 0.03, 1), baseColor, shadowAttenutation);

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
