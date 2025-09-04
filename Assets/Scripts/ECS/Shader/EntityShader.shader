Shader "Custom/EntityShader"
{
    Properties
    {
        _ColorTex ("Texture2D", 2D) = "white" {}
    }
    
    HLSLINCLUDE
    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
    #include "../../Shader/Util/WorldPos.cginc" 
	
    #pragma enable_d3d11_debug_symbols

    CBUFFER_START(UnityPerMaterial)
        sampler2D _ColorTex;
        float4 _Scale;
        uint _Type;
    CBUFFER_END

    ENDHLSL
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100
        Cull Off

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
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
                UNITY_VERTEX_INPUT_INSTANCE_ID
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float4 vertex : SV_POSITION;
                uint InstanceID : TEXCOORD1;
            };

            v2f vert (appdata v, uint InstanceID : SV_InstanceID)
            {
                v2f o;
                UNITY_SETUP_INSTANCE_ID(v);
                
                float3 Pos = v.vertex.xyz * _Scale.xyz + PositionBuffer[InstanceID];
                    
                VertexPositionInputs vertexInput = GetVertexPositionInputs(Pos);
                o.vertex = vertexInput.positionCS;
                o.uv = v.uv;
                o.InstanceID = InstanceID;
                return o;
            }

            
            float4 getRegularColor(v2f i){
                // as we have a split uv map we need to wrap around
                int xType = _Type / 16.0;
                int yType = _Type % 16;
                float StandardColor = (i.uv.x * 16.0) + xType * 8.0;
                float u = StandardColor;
                float v = yType;
                float2 uv = float2(u, v) / 16.0;
                float4 color = tex2D(_ColorTex, uv);
                return color;
            }

            half4 frag (v2f i) : SV_Target
            {
                return getRegularColor(i);
            }
            
            ENDHLSL
        }
    }
}
