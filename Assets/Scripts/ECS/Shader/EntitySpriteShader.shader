Shader "Custom/EntitySpriteShader"
{
    Properties
    {
        _MainTex ("Texture", 2D) = "white" {}
        _Scale ("Scale", Vector) = (0,0,0,0)
        // contains tex px width and height in x,y and count in z,w
        _Size ("Size", Vector) = (0,0,0,0)
    }
    
    HLSLINCLUDE
    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
    #include "../../Shader/Util/WorldPos.cginc" 
	
    #pragma enable_d3d11_debug_symbols

    CBUFFER_START(UnityPerMaterial)
        sampler2D _MainTex;
        float4 _CamOffset;
        float4 _Scale;
        float4 _Size;
    CBUFFER_END

    ENDHLSL

    SubShader
    {
        Tags { "RenderType"="Opaque" }
        //Tags { "RenderType"="Transparent" "LightMode" = "UniversalForward"  }
        LOD 100
        Cull Off
	    Blend SrcAlpha OneMinusSrcAlpha 

        Pass
        {
            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #pragma target 4.5

            #pragma multi_compile_instancing

            static float3 Vertices[] = {
                float3(-.5, 0, 0),
                float3(.5, 0, 0),
                float3(.5, 1, 0),
                float3(-.5, 1, 0),
            };

            StructuredBuffer<float3> PositionBuffer;
            StructuredBuffer<uint> TypeBuffer;

            struct appdata
            {
                uint vertexID : SV_VERTEXID;
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
                UNITY_VERTEX_INPUT_INSTANCE_ID
            };

            struct v2f
            {
                float2 uv : TEXCOORD0;
                float4 vertex : SV_POSITION;
            };
            

            float4 GetWorldPos(int VertexIndex, int InstanceID){
                float3 Vert = Vertices[VertexIndex];
                float3 Offset = 
                    Vert.x * _CamRight.xyz +
                    Vert.y * _CamUp.xyz;
                float3 Pos = PositionBuffer[InstanceID] + Offset * _Scale.xyz;
                return float4(Pos, 1);
            }

            v2f vert (appdata v, uint InstanceID : SV_InstanceID)
            {
                v2f o;
                float3 Pos = GetWorldPos(v.vertexID, InstanceID).xyz;
                o.vertex = TransformWorldToHClip(Pos);
                o.uv = v.uv;
                return o;
            }

            float4 frag (v2f i, uint InstanceID : SV_InstanceID) : SV_Target
            {
                uint Type = TypeBuffer[InstanceID];
                uint x = Type % _Size.z;
                uint y = Type / _Size.w;
                // cutout smaller picture from combined one
                float2 uv = (float2(x, y) * _Size.xy + i.uv * _Size.xy) / (_Size.xy * _Size.zw);
                float4 col = tex2D(_MainTex, uv);
                return col;
            }
            ENDHLSL
        }
    }
}
