Shader "Custom/Grass"
{
    Properties
    {
        _GrassTex ("Grass", 2D) = "white" {}
        _CopyTex ("CopyTex", 2D) = "white" {}
        _TargetTex ("TargetTex", 2D) = "white" {}
        _Cutoff("Cutoff", Float) = 0
        _Offset("Offset", Vector) = (0,0,0,0)
    }

    
    HLSLINCLUDE

    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"
    #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Lighting.hlsl" // shadows and ambient light
    #include "Util/GpuPrinter.cginc"
    #include "Util/WorldPos.cginc" 

    #pragma enable_d3d11_debug_symbols

    CBUFFER_START(UnityPerMaterial)
        sampler2D _GrassTex;
        sampler2D _CopyTex;
        sampler2D _TargetTex;
        
        sampler2D _CameraDepthTexture;
        float4 _CamOffset;
        float4 _Offset;
        float _Cutoff;
    CBUFFER_END

    ENDHLSL

    SubShader
    {
        Tags { "RenderType"="Transparent" "LightMode" = "UniversalForward"  }
        LOD 100
	    Blend SrcAlpha OneMinusSrcAlpha 
        ZTest LEqual
        //ZWrite Off

        Pass
        {
            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag
            #pragma target 4.5

            #pragma multi_compile_instancing

            
            struct RenderInfo{
	            uint2 Pixel;
	            float3 WorldMin;
	            float3 WorldMax;
	            float3 Pos;
            };

            StructuredBuffer<float3> PositionBuffer;
            
            struct appdata
            {
                uint vertexID : SV_VERTEXID;
                float4 vertex : POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;
                UNITY_VERTEX_INPUT_INSTANCE_ID
            };

            struct v2f
            {
                float4 vertex : SV_POSITION;
                float3 normal : NORMAL;
                float2 uv : TEXCOORD0;
                float2 screen : TEXCOORD1;
                float3 world : TEXCOORD2;
            };

            static float3 Vertices[] = {
                float3(-.5, 0, 0),
                float3(.5, 0, 0),
                float3(-.5, 1, 0),
                float3(.5, 1, 0),
            };
            
            v2f vert (appdata v, uint InstanceID : SV_INSTANCEID)
            {
                UNITY_SETUP_INSTANCE_ID(v);
                v2f o;
                float3 Vert = Vertices[v.vertexID];
                float3 Offset = 
                    Vert.x * _CamRight.xyz +
                    Vert.y * _CamUp.xyz;
                float3 Pos = PositionBuffer[InstanceID] + Offset * 2;
                    
                VertexPositionInputs vertexInput = GetVertexPositionInputs(Pos);
                VertexPositionInputs BaseVertexInput = GetVertexPositionInputs(PositionBuffer[InstanceID]);
                
                o.vertex = vertexInput.positionCS;
                o.normal = -_CamForward.xyz;
                o.uv  = v.uv * 2;
                o.world = BaseVertexInput.positionWS;
                o.screen = ComputeScreenPos(BaseVertexInput.positionCS).xy;
                return o;
            }

            float4 frag (v2f i) : SV_Target
            {
                float Depth = tex2D(_CameraDepthTexture, i.screen).x;
                float3 WorldDepthPos = ComputeWorldSpacePosition(i.screen, Depth, UNITY_MATRIX_I_VP);
                float Dis = distance(WorldDepthPos, i.world);
                clip(_Cutoff - Dis);

                float4 GrassColor = tex2D(_GrassTex, i.uv);
                clip(GrassColor.a - 0.5);
                float4 Color = tex2D(_CopyTex, i.screen);
                return float4(Color.xyz, 1);
            }
            ENDHLSL
        }
    
    }
}
