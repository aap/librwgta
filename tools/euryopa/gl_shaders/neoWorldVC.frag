#version 330

layout(std140) uniform State
{
	int   u_alphaTest;
	float u_alphaRef;

	int   u_fogEnable;
	float u_fogStart;
	float u_fogEnd;
	vec4  u_fogColor;
};

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform vec4 u_lm;

in vec4 v_color;
in vec2 v_tex0;
in vec2 v_tex1;
in float v_fog;

out vec4 color;

void
main(void)
{
	vec4 t0 = texture(tex0, vec2(v_tex0.x, v_tex0.y));
	vec4 t1 = texture(tex1, vec2(v_tex1.x, v_tex1.y));
	color = t0*v_color;
	color *= u_lm*t1 + 1-u_lm;
	color.a = v_color.a*t0.a*u_lm.a;
	if(u_fogEnable != 0)
		color.rgb = mix(u_fogColor.rgb, color.rgb, v_fog);
	switch(u_alphaTest){
	default:
	case 0: break;
	case 1:
		if(color.a < u_alphaRef)
			discard;
		break;
	case 2:
		if(color.a >= u_alphaRef)
			discard;
		break;
	}
}

