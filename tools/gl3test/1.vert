#version 330

layout(std140) uniform Scene
{
	mat4 u_proj;
	mat4 u_view;
};

layout(std140) uniform Object
{
	mat4 u_world;
	vec4 u_ambLight;
};

uniform vec4 u_matColor;
uniform vec4 u_surfaceProps;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_color;

out vec4 v_color;

void
main(void)
{
	vec3 lightdir = vec3(1.0, 1.0, -1.0);

	vec4 V = u_world * vec4(in_pos, 1.0);
	gl_Position = u_proj * u_view * V;
	vec3 N = mat3(u_world) * in_normal;

	float L = max(0.0, dot(N, -normalize(lightdir)));
	vec3 diff = vec3(L, L, L);

	v_color = in_color;
	v_color.rgb += diff + u_ambLight.rgb;
}

