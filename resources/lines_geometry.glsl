#version 410 core

layout(lines) in;
layout(line_strip, max_vertices = 2) out;
in vec3 vertex_pos[];
in vec2 vertex_tex[];

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 frag_pos;
out vec2 frag_tex;
out vec3 frag_norm;
void main()
{

 
	gl_Position = P * V * gl_in[0].gl_Position;
	frag_pos=gl_in[0].gl_Position.xyz;
	frag_tex=vertex_tex[0];
	frag_norm = vec3(0,1,0);
	EmitVertex();
	gl_Position = P * V * gl_in[1].gl_Position;
	frag_pos=gl_in[1].gl_Position.xyz;
	frag_tex=vertex_tex[1];
	frag_norm = vec3(0,1,0);
	EmitVertex();


   // EndPrimitive();
}