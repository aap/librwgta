const char *neoWorldIII_frag_src =
"#version 330\n"

"layout(std140) uniform State\n"
"{\n"
"	int   u_alphaTest;\n"
"	float u_alphaRef;\n"

"	int   u_fogEnable;\n"
"	float u_fogStart;\n"
"	float u_fogEnd;\n"
"	vec4  u_fogColor;\n"
"};\n"

"uniform sampler2D tex0;\n"
"uniform sampler2D tex1;\n"
"uniform vec4 u_lm;\n"

"in vec4 v_color;\n"
"in vec2 v_tex0;\n"
"in vec2 v_tex1;\n"
"in float v_fog;\n"

"out vec4 color;\n"

"void\n"
"main(void)\n"
"{\n"
"	vec4 t0 = texture(tex0, vec2(v_tex0.x, v_tex0.y));\n"
"	vec4 t1 = texture(tex1, vec2(v_tex1.x, v_tex1.y));\n"
"	color = t0*v_color*(1 + u_lm*(2*t1-1));\n"
"	color.a = v_color.a*t0.a*u_lm.a;\n"
"	if(u_fogEnable != 0)\n"
"		color.rgb = mix(u_fogColor.rgb, color.rgb, v_fog);\n"
"	switch(u_alphaTest){\n"
"	default:\n"
"	case 0: break;\n"
"	case 1:\n"
"		if(color.a < u_alphaRef)\n"
"			discard;\n"
"		break;\n"
"	case 2:\n"
"		if(color.a >= u_alphaRef)\n"
"			discard;\n"
"		break;\n"
"	}\n"
"}\n"

;
