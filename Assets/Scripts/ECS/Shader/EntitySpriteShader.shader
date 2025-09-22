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
    #include "../../Shader/Util/EntitySpriteUtil.cginc" 
	
    #pragma enable_d3d11_debug_symbols

    ENDHLSL

    SubShader
    {
        Tags { "RenderType"="Transparent" "LightMode" = "UniversalForward"  }
        LOD 100
        Cull Off
	    Blend SrcAlpha OneMinusSrcAlpha 

        Pass
        {
            Name "Instanciated"
            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #pragma target 4.5

            #pragma multi_compile_instancing


            static float3 TextOffset = float3(.6, -.1, 0);
             
            static float3 Vertices[] = {
                float3(-.5, 0, 0),
                float3(.5, 0, 0),
                float3(.5, 1, 0),
                float3(-.5, 1, 0),
            };

            struct appdata
            {
                uint vertexID : SV_VERTEXID;
                float4 vertex : POSITION;
                float2 uv : TEXCOORD0;
                UNITY_VERTEX_INPUT_INSTANCE_ID
            };

            struct Item{
                uint Type;
                uint Amount;
            };

            StructuredBuffer<float3> PositionBuffer;
            StructuredBuffer<Item> ItemBuffer;

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
                float3 Pos = GetWorldPos(v.vertexID, InstanceID).xyz;
                float3 MidPoint = (GetWorldPos(0, InstanceID) + GetWorldPos(2, InstanceID)) / 2.0 + TextOffset;
                return GetV2F(Pos, MidPoint, v.uv);
            }

            float4 frag (v2f i, uint InstanceID : SV_InstanceID) : SV_Target
            {
                uint Amount = ItemBuffer[InstanceID].Amount;
                uint Type = ItemBuffer[InstanceID].Type;
                return GetColor(Type, Amount, i);
            }

            ENDHLSL
        }
    }
}
