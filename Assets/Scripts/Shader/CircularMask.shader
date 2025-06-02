Shader "Custom/CircularMask"
{
    Properties
    {
        _PosSize ("Pos & Size", vector) = (0,0,0,0)
        _Cutoff("Cutoff", Range(0, 1)) = 0
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
            #include "UnityUI.cginc"

            #define IPI 0.31830988618 

            struct appdata
            {
                float4 vertex : POSITION;
                float4 color : COLOR;
            };

            struct v2f
            {
                float4 vertex : SV_POSITION;
                float4 color : COLOR;
                float3 normal : NORMAL;
            };

            float4 _PosSize;
            float _Cutoff;

            v2f vert (appdata v)
            {
                v2f o;
                o.vertex = UnityObjectToClipPos(v.vertex);
                o.color = v.color;
                o.normal = v.vertex;
                return o;
            }

            fixed4 frag (v2f i) : SV_Target
            {
                // bring from screen space to -0.5..0.5 
                float2 uv = (i.normal.xy - _PosSize.xy)/ _PosSize.zw;
                // get inverse angle, but rotate 90°. Range from 0..1
                float angle = 1 - (atan2(uv.x, -uv.y) * IPI + 1) / 2;
                //return float4(angle < _Cutoff, 0, 0, 1);
                float3 gray = float3(0.21, 0.71, 0.07);
                float grayScale = dot(gray, i.color);
                float3 color = angle > _Cutoff ? grayScale : i.color;
                return float4(color, i.color.a);
            }
            ENDCG
        }
    }
}
