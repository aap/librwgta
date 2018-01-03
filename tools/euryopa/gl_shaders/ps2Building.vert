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

uniform vec4 u_matColor;
uniform vec4 u_surfaceProps;	// amb, spec, diff, extra
uniform vec4 u_dayparam;
uniform vec4 u_nightparam;
uniform float u_colorscale;

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec4 in_nightcolor;
layout(location = 3) in vec2 in_tex0;
layout(location = 4) in vec4 in_daycolor;

out vec4 v_color;
out vec2 v_tex0;
out float v_fog;

void
main(void)
{
	vec4 V = u_world * vec4(in_pos, 1.0);
	vec4 cV = u_view * V;
	gl_Position = u_proj * cV;

	v_color = in_nightcolor*u_nightparam + in_daycolor*u_dayparam;
	v_color *= u_matColor / u_colorscale;
	v_color.rgb += u_ambLight.rgb*u_surfaceProps.x;

	v_tex0 = in_tex0;

	v_fog = clamp((cV.z - u_fogEnd)/(u_fogStart - u_fogEnd), 0.0, 1.0);
}
