/**
    Fullscreen shader to display the previous, small RT onto th screen, upscaling the
    pixelized image. Does not need an actual object, but should be assigned to a FullScreenRenderPass
    Uses Normal/Depth to calculate EdgeHighlighting
*/
Shader "Custom/FullscreenPixelize"
{
    Properties
    {
        _Highlight("Highlight", Float) = 1
        [Header(Depth)][Space]
        _DepthWidth("Width", Float) = 1
        _DepthStrength("Strength", Float) = 1
        _DepthThreshold("Threshold", Range(0, 1)) = 0.01
        [Header(Normal)][Space]
        _NormalWidth("Width", Float) = 1
        _NormalStrength("Strength", Float) = 1
        _NormalThreshold("Threshold", Range(0, 1)) = 0.01

        [Header(Debug)][Space]
        _Debug("Debug", Float) = 0
    }

    
    HLSLINCLUDE
    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Lighting.hlsl" // shadows and ambient light
    #include "Util/WorldPos.cginc"
    #include "Util/GpuPrinter.cginc"
    
    struct Attributes {
        uint vertexID : SV_VertexID;
    };


    CBUFFER_START(UnityPerMaterial)
        sampler2D _CameraNormalsTexture;
        sampler2D _CameraDepthTexture;
        sampler2D _MainTex;
        float4 _MainTex_TexelSize;
        float _DepthWidth;
        float _DepthStrength;
        float _DepthThreshold;
        float _NormalWidth;
        float _NormalStrength;
        float _NormalThreshold;
        float _Highlight;
        float _Debug;
    CBUFFER_END

    


    ENDHLSL
    SubShader
    {
        Tags { "RenderType"="Opaque" "RenderPipeline" = "UniversalPipeline" "LightMode" = "UniversalForward"}
        LOD 100
        ZTest LEqual

        Pass
        {
            Name "Fullscreen"
            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            struct appdata
            {
                uint vertexID : SV_VertexID;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float4 vertex : SV_POSITION;
            };

            v2f vert (appdata v)
            {
                v2f o;
                o.vertex = GetFullScreenTriangleVertexPosition(v.vertexID);
                o.uv  = GetFullScreenTriangleTexCoord(v.vertexID);
                return o;
            }

            void GetUVs(float2 uv, float Width, out float2 TopRightUV, out float2 BottomRightUV,
                    out float2 BottomLeftUV, out float2 TopLeftUV){
                float halfScaleFloor = floor(Width * 0.5);
                float halfScaleCeil = ceil(Width * 0.5);

                BottomLeftUV = uv - float2(_MainTex_TexelSize.x, _MainTex_TexelSize.y) * halfScaleFloor;
                TopRightUV = uv + float2(_MainTex_TexelSize.x, _MainTex_TexelSize.y) * halfScaleCeil;  
                BottomRightUV = uv + float2(_MainTex_TexelSize.x * halfScaleCeil, -_MainTex_TexelSize.y * halfScaleFloor);
                TopLeftUV = uv + float2(-_MainTex_TexelSize.x * halfScaleFloor, _MainTex_TexelSize.y * halfScaleCeil);   
            }

            float2 Quantize(float2 uv){
                // Quantize uv to imitate pixel, otherwise it will be a smooth outline
                float2 size = float2(455, 256);
                return int2(uv * size) / size;
            }

            float SampleNormal(float2 uv){
                float2 TopRightUV, BottomRightUV, BottomLeftUV, TopLeftUV;
                GetUVs(uv, _NormalWidth, TopRightUV, BottomRightUV, BottomLeftUV, TopLeftUV);
                
                float3 n0 = tex2D(_CameraNormalsTexture, TopRightUV);
                float3 n1 = tex2D(_CameraNormalsTexture, BottomRightUV);
                float3 n2 = tex2D(_CameraNormalsTexture, BottomLeftUV);
                float3 n3 = tex2D(_CameraNormalsTexture, TopLeftUV);
                float3 nd0 = abs(n2 - n0);
                float3 nd1 = abs(n3 - n1);

                float edge = sqrt(dot(nd0, nd0) + dot(nd1, nd1));
                return (edge * _NormalStrength) > _NormalThreshold ? 1 : 0;
            }

            float SampleDepth(float2 uv){
                float2 TopRightUV, BottomRightUV, BottomLeftUV, TopLeftUV;
                GetUVs(uv, _DepthWidth, TopRightUV, BottomRightUV, BottomLeftUV, TopLeftUV);

                float d = tex2D(_CameraDepthTexture, uv).r;
                float d0 = tex2D(_CameraDepthTexture, TopRightUV).r;
                float d1 = tex2D(_CameraDepthTexture, BottomRightUV).r;
                float d2 = tex2D(_CameraDepthTexture, BottomLeftUV).r;
                float d3 = tex2D(_CameraDepthTexture, TopLeftUV).r;
                float dd0 = abs(d2 - d0);
                float dd1 = abs(d3 - d1);

                //robert cross algo
                float edge = sqrt(dd0 * dd0 + dd1 * dd1);
                return (edge * _DepthStrength) > _DepthThreshold ? 1 : 0;
            }

            half4 frag (v2f i) : SV_Target
            {   
                float2 uv = Quantize(i.uv);
                float4 Color = tex2D(_MainTex, uv);
                float Edge = max(SampleDepth(uv), SampleNormal(uv));
                float Desaturated = dot(Color.xyz, float3(0.2126, 0.7152, 0.0722));
                Color.xyz += (1 - Desaturated) * Edge * _Highlight;
                return Color;
                //float NeighbourDepth = tex2D(_CameraDepthTexture, NeighbourUV).x;
                
            }
            ENDHLSL
        }
    }
}
