const char *leedsBuilding_vert_src =
"uniform vec4 u_amb;\n"
"uniform vec4 u_emiss;\n"

"#define surfEmissive (u_surfProps.w)\n"

"VSIN(ATTRIB_POS)	vec3 in_pos;\n"

"VSOUT vec4 v_color;\n"
"VSOUT vec2 v_tex0;\n"
"VSOUT float v_fog;\n"

"void\n"
"main(void)\n"
"{\n"
"	vec4 Vertex = u_world * vec4(in_pos, 1.0);\n"
"	gl_Position = u_proj * u_view * Vertex;\n"
"	vec3 Normal = mat3(u_world) * in_normal;\n"

"	v_tex0 = in_tex0;\n"

"	v_color = in_color;\n"
"	v_color.rgb *= u_amb.rgb;\n"
"	v_color.rgb += u_emiss.rgb*surfEmissive;\n"
"	v_color = clamp(v_color, 0.0, 1.0);\n"
"	v_color.a *= u_matColor.a;\n"

"	v_fog = DoFog(gl_Position.w);\n"
"}\n"
;
