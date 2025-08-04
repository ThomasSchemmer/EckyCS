Shader "TerrainTool/TTTerain"
{
    Properties
    {
        [Header(Base)][Space]
        _HeightTex ("Height Tex", 2D) = "white" {}
        _GrassColor("Grass", Color) = (0,0,0,0)
        _RockColor("Rock", Color) = (0,0,0,0)
        _Quantize("Quantize", Float) = 1
        _GrassThreshold("Grass Threshold", Range(0, 1)) = 1
        
        [Header(Grass)][Space]
        _GrassScale("Scale", Float) = 1
        _GrassQuantize("Quantize", Float) = 1
    }

    
    HLSLINCLUDE

    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Lighting.hlsl" // shadows and ambient light
    #include "Perlin.cginc"
    #include "Cubic.cginc"

    CBUFFER_START(UnityPerMaterial)
        sampler2D _HeightTex;
        
        float4 _GrassColor;
        float4 _RockColor;
        float _Quantize;
        float _GrassThreshold;
        float4 _MousePosition;

        // Grass
        float _GrassScale;
        float _GrassQuantize;
    CBUFFER_END
    
    StructuredBuffer<float3> PositionBuffer;
    uniform float _Width;
    
    struct v2g
    {
        float4 vertex : SV_POSITION;
        float2 uv : TEXCOORD0;
        float3 world : TEXCOORD1;
    };

    struct g2f
    {
        float4 vertex : SV_POSITION;
        float2 uv : TEXCOORD0;
        float3 normal : NORMAL;
        float3 world : TEXCOORD1;
    };

    
    v2g vert (uint id : SV_VERTEXID)
    {
        v2g o;
        VertexPositionInputs vertexInput = GetVertexPositionInputs(PositionBuffer[id]);
        o.vertex = vertexInput.positionCS;
        o.world = vertexInput.positionWS;
        o.uv = o.world.xz / _Width;
        return o;
    }
            
    [maxvertexcount(3)]
    void geom(triangle v2g IN[3], inout TriangleStream<g2f> triStream){
                
    
        float3 AB = (IN[1].world - IN[0].world);
        float3 AC = (IN[2].world - IN[0].world);
        float3 triangleNormal = normalize(cross(AB, AC));
     
        [unroll]
        for (int i = 0; i < 3; i++)
        {
            g2f o;
            o.vertex = IN[i].vertex;
            o.uv = IN[i].uv;
            o.world = IN[i].world;
            o.normal = triangleNormal;
        
            triStream.Append(o);
        }

        triStream.RestartStrip();
    }


    ENDHLSL

    SubShader
    {
        Tags { "RenderType"="Opaque" "LightMode" = "UniversalForward"  }
        LOD 100
        Cull Off

        Pass
        {
            HLSLPROGRAM
            #pragma vertex vert
            #pragma geometry geom
            #pragma fragment frag
            
            #pragma multi_compile _ _MAIN_LIGHT_SHADOWS
            #pragma multi_compile _ _MAIN_LIGHT_SHADOWS_CASCADE
            #pragma multi_compile _ _SHADOWS_SOFT

            #define BRUSHBORDER 0.25

            
            uniform float _BrushSize;


            float3 GetBrushColor(g2f i){
                float d = distance(i.world, _MousePosition.xyz);
                float a0 = smoothstep(_BrushSize - BRUSHBORDER, _BrushSize, d);
                float a1 = smoothstep(_BrushSize, _BrushSize + BRUSHBORDER, d);
                float3 BrushColor = a0 - a1;
                return BrushColor;
            }

            float GetLight(g2f i){
            
                Light light = GetMainLight();
                float nl = max(0, dot(i.normal, light.direction.xyz));
                nl = (int)(nl * _Quantize) / _Quantize;
                return nl;
            }
            
            float GetShadow(float3 WorldPos){
                VertexPositionInputs vertexInput = (VertexPositionInputs)0;
                vertexInput.positionWS = WorldPos;
                float4 shadowCoord = GetShadowCoord(vertexInput);
                half shadowAttenutation = MainLightRealtimeShadow(shadowCoord);
                return shadowAttenutation;
            }

            float4 frag (g2f i, bool bIsFront : SV_IsFrontFace) : SV_Target
            {   
                float nl = GetLight(i);
                float GrassNoise = abs(cubicNoise(i.world.xyz * _GrassScale));
                GrassNoise += abs(cubicNoise(-i.world.xyz * _GrassScale * 2)) * 0.5;
                GrassNoise = (int)(GrassNoise * _GrassQuantize) / _GrassQuantize;
                GrassNoise = saturate(GrassNoise);
                nl += GrassNoise;

                float RockFactor = abs(dot(i.normal, float3(1, 0, 0)) + dot(i.normal, float3(0, 0, 1)));  
                float GrassFactor = 1 - RockFactor;//dot(i.normal, float3(0,1,0));

                float3 Color = GrassFactor * _GrassColor + RockFactor * _RockColor;
                Color = Color * (bIsFront ? 1 : -1);
                Color *= nl;

                float3 BrushColor = GetBrushColor(i);
                float3 TotalColor = BrushColor + Color;
                
                float shadowAttenutation = GetShadow(i.world);
                TotalColor = lerp(float3(0.01, 0.01, 0.03), TotalColor, shadowAttenutation);
                return float4(TotalColor, 1);
            }
            ENDHLSL
        }

        
        // DepthNormalsOnly Pass
        Pass
        {
            Tags { "RenderType"="Opaque"  "LightMode" = "DepthNormalsOnly" }
            
            HLSLPROGRAM
            #pragma vertex vert
            #pragma geometry geom
            #pragma fragment DepthNormalsFragment

            #include "Packages/com.unity.render-pipelines.universal/Shaders/LitInput.hlsl"


            half4 DepthNormalsFragment(g2f i, out float Depth : SV_DEPTH) : SV_TARGET
            {
                Depth = ComputeNormalizedDeviceCoordinatesWithZ(i.world, GetWorldToHClipMatrix()).z;
                
                return float4(i.normal * 0.5 + 0.5, 1);
            }
            ENDHLSL
        }
    }
    FallBack "Mobile/Diffuse"
}
