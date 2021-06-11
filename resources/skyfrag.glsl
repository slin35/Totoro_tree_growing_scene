#version 330 core
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;

uniform sampler2D tex;
uniform vec3 camPos;

out vec4 fragCol;

void main()
{
	vec4 tcol = texture(tex, vertex_tex);
	fragCol = tcol;
	fragCol.rgb *= 0.4;
}



