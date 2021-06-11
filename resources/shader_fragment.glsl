#version 410 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform vec2 texoff;
uniform vec2 texoff_last;
uniform float t;

uniform sampler2D tex;
uniform sampler2D tex2;

void main()
{

	//vec4 tcol = texture(tex, vertex_tex/4 + texoff/4);
	vec4 tcol = texture(tex, vec2(vertex_tex.x / 4, vertex_tex.y) + vec2(texoff.x / 4, texoff.y));
	vec4 tcol2 = texture(tex, vec2(vertex_tex.x / 4, vertex_tex.y) + vec2(texoff_last.x / 4, texoff_last.y));

	//color = tcol;
	color = (1 - t) * tcol + t * tcol2;


}