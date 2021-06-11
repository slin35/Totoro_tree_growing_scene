#version 410 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;
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
vec3 ab = (gl_in[0].gl_Position - gl_in[1].gl_Position).xyz;
vec3 ac = (gl_in[0].gl_Position - gl_in[2].gl_Position).xyz;
vec3 c = normalize(cross(ab,ac));

 for(int i = 0; i < gl_in.length(); i++)
  {
	gl_Position = P * V * gl_in[i].gl_Position;
	frag_pos=gl_in[i].gl_Position.xyz;
	frag_tex=vertex_tex[i];
	frag_norm = c;
	EmitVertex();
}

   // EndPrimitive();
}