const char *colcode_vert_src =
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

"layout(std140) uniform Scene\n"
"{\n"
"	mat4 u_proj;\n"
"	mat4 u_view;\n"
"};\n"

"#define MAX_LIGHTS 8\n"
"struct Light {\n"
"	vec4  position;\n"
"	vec4  direction;\n"
"	vec4  color;\n"
"	float radius;\n"
"	float minusCosAngle;\n"
"};\n"

"layout(std140) uniform Object\n"
"{\n"
"	mat4  u_world;\n"
"	vec4  u_ambLight;\n"
"	int   u_numLights;\n"
"	Light u_lights[MAX_LIGHTS];\n"
"};\n"

"uniform vec4 u_matColor;\n"

"layout(location = 0) in vec3 in_pos;\n"
"layout(location = 1) in vec3 in_normal;\n"
"layout(location = 2) in vec4 in_color;\n"
"layout(location = 3) in vec2 in_tex0;\n"

"out vec4 v_color;\n"
"out vec2 v_tex0;\n"

"void\n"
"main(void)\n"
"{\n"
"	vec4 V = u_world * vec4(in_pos, 1.0);\n"
"	vec4 cV = u_view * V;\n"
"	gl_Position = u_proj * cV;\n"

"	v_color = u_matColor;\n"
"	v_tex0 = in_tex0;\n"
"}\n"
;
