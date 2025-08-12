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
    CBUFFER_END

    ENDHLSL
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        Pass
        {
            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #pragma multi_compile_instancing

            //#define UNITY_INDIRECT_DRAW_ARGS IndirectDrawIndexedArgs
            //#include "UnityIndirect.cginc"
            
            shared StructuredBuffer<float3> _Positions;
            shared StructuredBuffer<uint> _IDs;
            float _Count;

            struct appdata
            {
                float4 vertex : POSITION;
                uint vertexID : SV_VERTEXID;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float4 vertex : SV_POSITION;
                uint ID : NORMAL;
            };


            int IsInvalid(uint ID){
                return (ID & 0x000000FF) == 255 ? 1 : 0;
            }
            

            static float3 Vertices[] = {
                float3(-.5, 0, 0),
                float3(.5, 0, 0),
                float3(-.5, 1, 0),
                float3(.5, 1, 0),
            };

            v2f vert (appdata v, uint InstanceID : SV_InstanceID)
            {
                UNITY_SETUP_INSTANCE_ID(v);
                v2f o;
                
                float3 Vert = Vertices[v.vertexID];
                
                float3 Offset = 
                    Vert.x * _CamRight.xyz +
                    Vert.y * _CamUp.xyz;
                float3 Pos = _Positions[InstanceID] + Offset * 2;
                    
                VertexPositionInputs vertexInput = GetVertexPositionInputs(Pos);
                o.vertex = vertexInput.positionCS;
                //float3 offset = v.vertex.x * _CamForward + v.vertex.y * _CamUp;
                //float4 vertex = float4(_Positions[svInstanceID], 1) + float4(offset, 1);
                //
                //o.vertex = GetVertexPositionInputs(vertex.xyz).positionCS;
                //float3 Temp = (_CamPos - _Positions[svInstanceID]) * (_CamPos - _Positions[svInstanceID]);
               // o.vertex.w -= 1;//*= distance(_Positions[instanceID], _CamPos);//Temp.x + Temp.y + Temp.z;
                o.uv = TRANSFORM_TEX(v.uv, _MainTex);
                o.ID = _IDs[InstanceID];
                return o;
            }

            half4 frag (v2f i) : SV_Target
            {
                return 1;
                clip(IsInvalid(i.ID));
                //fixed4 col = tex2D(_MainTex, i.uv);
                return half4(i.ID / _Count, 0, 0, 1);
            }
            
            ENDHLSL
        }
    }
}
