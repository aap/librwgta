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

layout(std140) uniform Scene
{
	mat4 u_proj;
	mat4 u_view;
};

#define MAX_LIGHTS 8
struct Light {
	vec4  position;
	vec4  direction;
	vec4  color;
	float radius;
	float minusCosAngle;
};

layout(std140) uniform Object
{
	mat4  u_world;
	vec4  u_ambLight;
	int   u_numLights;
	Light u_lights[MAX_LIGHTS];
};

uniform mat4  u_envmat;
uniform vec4  u_envXform;
uniform float u_shininess;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;

out vec4 v_color;
out vec2 v_tex0;

void
main(void)
{
	gl_Position = u_proj * u_view * u_world * vec4(in_pos, 1.0);
	v_color = vec4(192.0, 192.0, 192.0, 0.0)/128.0*u_shininess;
	v_tex0 = (u_envmat * vec4(in_normal, 0.0f)).xy - u_envXform.xy;
	v_tex0 *= u_envXform.zw;
}
