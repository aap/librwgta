float4x4	combined	: register(c0);
float3		ambient		: register(c4);
float3		emissive	: register(c5);
float4		matCol		: register(c19);
float4		surfProps	: register(c20);

#define surfAmb (surfProps.x)
#define surfEmiss (surfProps.w)

struct VS_INPUT
{
	float4 Position		: POSITION;
	float2 TexCoord		: TEXCOORD0;
	float4 Color		: COLOR0;
};

struct VS_OUTPUT {
	float4 Position		: POSITION;
	float2 Texcoord0	: TEXCOORD0;
	float4 Color		: COLOR0;
};

VS_OUTPUT main(in VS_INPUT IN)
{
	VS_OUTPUT OUT;

	OUT.Position = mul(combined, IN.Position);
	OUT.Texcoord0 = IN.TexCoord;

	OUT.Color = saturate(IN.Color*float4(ambient, 1.0f) + float4(emissive, 0.0f)*surfEmiss);
	OUT.Color.w *= matCol.w;

	return OUT;
}
