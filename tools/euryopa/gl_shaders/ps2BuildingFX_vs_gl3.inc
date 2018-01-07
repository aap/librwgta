const char *ps2BuildingFX_vert_src =
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

"uniform mat4  u_envmat;\n"
"uniform vec4  u_envXform;\n"
"uniform float u_shininess;\n"

"layout(location = 0) in vec3 in_pos;\n"
"layout(location = 1) in vec3 in_normal;\n"

"out vec4 v_color;\n"
"out vec2 v_tex0;\n"

"void\n"
"main(void)\n"
"{\n"
"	gl_Position = u_proj * u_view * u_world * vec4(in_pos, 1.0);\n"
"	v_color = vec4(192.0, 192.0, 192.0, 0.0)/128.0*u_shininess;\n"
"	v_tex0 = (u_envmat * vec4(in_normal, 0.0f)).xy - u_envXform.xy;\n"
"	v_tex0 *= u_envXform.zw;\n"
"}\n"
;
