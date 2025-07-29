Shader "TerrainTool/TTTerain"
{
    Properties
    {
        _HeightTex ("Height Tex", 2D) = "white" {}
    }

    
    HLSLINCLUDE

    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"


    CBUFFER_START(UnityPerMaterial)
        sampler2D _HeightTex;

        float4 _MousePosition;
    CBUFFER_END

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

            #define BRUSHBORDER 0.25

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

            StructuredBuffer<float3> PositionBuffer;
            uniform float _Width;
            uniform float _BrushSize;

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
                
    
                float3 AB = normalize(IN[1].world - IN[0].world);
                float3 AC = normalize(IN[2].world - IN[0].world);
                float3 triangleNormal = cross(AB, AC);
     
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

            float4 frag (g2f i, bool bIsFront : SV_IsFrontFace) : SV_Target
            {   
                float d = distance(i.world, _MousePosition.xyz);
                float a0 = smoothstep(_BrushSize - BRUSHBORDER, _BrushSize, d);
                float a1 = smoothstep(_BrushSize, _BrushSize + BRUSHBORDER, d);
                float3 BrushColor = a0 - a1;
                float3 Color = i.normal * (bIsFront ? 1 : -1);  //tex2D(_HeightTex, i.uv);
                return float4(BrushColor + Color, 1);
            }
            ENDHLSL
        }
    }
}
