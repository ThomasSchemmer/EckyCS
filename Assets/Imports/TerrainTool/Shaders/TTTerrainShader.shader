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
            #pragma fragment frag

            #define BRUSHBORDER 0.25

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float4 vertex : SV_POSITION;
                float2 uv : TEXCOORD0;
                float3 world : TEXCOORD1;
            };

            StructuredBuffer<float3> PositionBuffer;
            uniform float _Width;
            uniform float _BrushSize;

            v2f vert (uint id : SV_VERTEXID)
            {
                v2f o;
                VertexPositionInputs vertexInput = GetVertexPositionInputs(PositionBuffer[id]);
                o.vertex = vertexInput.positionCS;
                o.world = vertexInput.positionWS;
                o.uv = o.world.xz / _Width;
                return o;
            }

            float4 frag (v2f i) : SV_Target
            {   
                //return float4(_MousePosition.xyz, 1);
                float d = distance(i.world, _MousePosition.xyz);
                float a0 = smoothstep(_BrushSize - BRUSHBORDER, _BrushSize, d);
                float a1 = smoothstep(_BrushSize, _BrushSize + BRUSHBORDER, d);
                float3 BrushColor = a0 - a1;
                float3 Color = tex2D(_HeightTex, i.uv);
                return float4(BrushColor + Color, 1);
            }
            ENDHLSL
        }
    }
}
