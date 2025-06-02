Shader "Custom/SpriteShader"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
    }
    SubShader
    {
        Tags { "RenderType"="Opaque" }
        LOD 100

        Pass
        {
            CGPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #include "UnityCG.cginc"
            #define UNITY_INDIRECT_DRAW_ARGS IndirectDrawIndexedArgs
            #include "UnityIndirect.cginc"

            struct appdata
            {
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float4 vertex : SV_POSITION;
                float ID : NORMAL;
            };

            sampler2D _MainTex;
            float4 _MainTex_ST;
            StructuredBuffer<float3> _Positions;
            float _Count;

            v2f vert (appdata v, uint svVertexID: SV_VertexID, uint svInstanceID : SV_InstanceID)
            {
                InitIndirectDrawArgs(0);
                uint instanceID = GetIndirectInstanceID(svInstanceID);
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex) + float4(_Positions[instanceID], 1);
                o.uv = TRANSFORM_TEX(v.uv, _MainTex);
                o.ID = instanceID;
                return o;
            }

            fixed4 frag (v2f i) : SV_Target
            {
                clip(_Count - i.ID);
                //fixed4 col = tex2D(_MainTex, i.uv);
                return float4(i.ID / _Count, 0, 0, 1);
            }
            ENDCG
        }
    }
}
