Shader "Custom/SpriteShader"
{
    Properties
    {
        _MainTex ("Texture2D", 2D) = "white" {}
    }
    
    HLSLINCLUDE
    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
    #include "../../Shader/Util/WorldPos.cginc" 
	
    CBUFFER_START(UnityPerMaterial)
    
        TEXTURE2D(_MainTex);
        SAMPLER(sampler_MainTex);
        float4 _MainTex_ST;
        float4 _Color;
        float4 _Scale;
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
                uint vertexID : SV_VERTEXID;
                float2 uv : TEXCOORD0;
                UNITY_VERTEX_INPUT_INSTANCE_ID
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float4 vertex : SV_POSITION;
                uint InstanceID : TEXCOORD1;
            };
                   
            static float3 Vertices[] = {
                float3(-.5, 0, 0),
                float3(.5, 0, 0),
                float3(-.5, 1, 0),
                float3(.5, 1, 0),
            };

            v2f vert (appdata v, uint InstanceID : SV_InstanceID)
            {
                v2f o;
                UNITY_SETUP_INSTANCE_ID(v);
                
                float3 Vert = Vertices[v.vertexID];
                
                float3 Offset = 
                    Vert.x * _CamRight.xyz +
                    Vert.y * _CamUp.xyz;
                float3 Pos = v.vertex.xyz * _Scale.xyz + PositionBuffer[InstanceID];
                    
                VertexPositionInputs vertexInput = GetVertexPositionInputs(Pos);
                o.vertex = vertexInput.positionCS;
                o.uv = TRANSFORM_TEX(v.uv, _MainTex);
                o.InstanceID = InstanceID;
                return o;
            }

            half4 frag (v2f i) : SV_Target
            {
                return float4(_Scale.xyz, 1);
                //fixed4 col = tex2D(_MainTex, i.uv);
                return half4(1, 0, 0, 1);
            }
            
            ENDHLSL
        }
    }
}
