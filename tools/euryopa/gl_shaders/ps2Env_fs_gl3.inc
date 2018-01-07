const char *ps2Env_frag_src =
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

"uniform sampler2D tex;\n"

"in vec4 v_color;\n"
"in vec2 v_tex0;\n"

"out vec4 color;\n"

"void\n"
"main(void)\n"
"{\n"
"	color = v_color*texture(tex, vec2(v_tex0.x, v_tex0.y));\n"
"	color.a = 1.0;\n"
"}\n"

;
