uniform sampler2D tex : register(s0);
uniform float4 blurcol : register(c0);


float4 main(in float2 texcoord : TEXCOORD0) : COLOR0
{
	float4 dst = tex2D(tex, texcoord.xy);
	dst.rgb *= float3(1.0, 1.0, 1.0)-blurcol.rgb;
	dst.a = 1.0;
	return dst;
}
