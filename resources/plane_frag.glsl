#version 330 core
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;

uniform vec3 camPos;
uniform sampler2D tex;

out vec4 color;

void main()
{
   vec3 lightpos  = vec3(100);
   vec3 lightdir  = normalize(lightpos - vertex_pos);
   vec3 camdir    = normalize(camPos - vertex_pos);
   vec3 frag_norm = normalize(vertex_normal);

   float diffuse_fact = clamp(dot(lightdir, frag_norm), 0, 1);

   vec3 h = normalize(camdir + lightdir);
   float spec_fact = clamp(dot(h, frag_norm), 0, 1);

   color.rgb = vec3(1) * 0.1 + vec3(0.4) * diffuse_fact + vec3(1) * spec_fact;
   color.a=1;
}


