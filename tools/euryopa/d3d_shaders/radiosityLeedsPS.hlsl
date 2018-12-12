uniform sampler2D tex : register(s0);
uniform float3 params : register(c2);

#define limit (params.x)
#define intensity (params.y)

float4 main(in float2 texcoord : TEXCOORD0) : COLOR0
{
	float4 fb = tex2D(tex, texcoord);
	fb.rgb += saturate(fb.rgb - float3(limit/2, limit/2, limit/2))*intensity*4;
	fb.a = 1.0f;
	return fb;
}
