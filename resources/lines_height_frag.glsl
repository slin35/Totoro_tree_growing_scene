#version 410 core
out vec4 color;
in vec3 frag_pos;
in vec2 frag_tex;
in vec3 frag_norm;

uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec3 camoff;
uniform vec3 campos;
uniform vec3 bgcolor;
void main()
{



float len = length(frag_pos.xz+campos.xz);
len-=350;
len/=140.;
len=clamp(len,0,1);

color.a=1;
color.rgb = vec3(0.5,0.4,1.0) * (1 - len) + bgcolor*len;

}
