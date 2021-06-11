#version 410 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec2 vertTex;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
out vec3 vertex_pos;
out vec2 vertex_tex;
uniform sampler2D tex;
uniform sampler2D tex2;
uniform vec3 camoff;

float hash(float n) { return fract(sin(n) * 753.5453123); }
float snoise(vec3 x)
	{
	vec3 p = floor(x);
	vec3 f = fract(x);
	f = f * f * (3.0 - 2.0 * f);

	float n = p.x + p.y * 157.0 + 113.0 * p.z;
	return mix(mix(mix(hash(n + 0.0), hash(n + 1.0), f.x),
		mix(hash(n + 157.0), hash(n + 158.0), f.x), f.y),
		mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
			mix(hash(n + 270.0), hash(n + 271.0), f.x), f.y), f.z);
	}
//Changing octaves, frequency and presistance results in a total different landscape.
float noise(vec3 position, int octaves, float frequency, float persistence) {
	float total = 0.0;
	float maxAmplitude = 0.0;
	float amplitude = 1.0;
	for (int i = 0; i < octaves; i++) {
		total += snoise(position * frequency) * amplitude;
		frequency *= 2.0;
		maxAmplitude += amplitude;
		amplitude *= persistence;
		}
	return total / maxAmplitude;
	}

void main()
{

	vec2 texcoords=vertTex;
	float t=1./1000.;
	texcoords -= vec2(camoff.x,camoff.z)*t;
	float texheight = texture(tex, texcoords*.3).r;

	vec4 tpos =  vec4(vertPos, 1.0);
	tpos.z -=camoff.z;
	tpos.x -=camoff.x;
	
	float height = noise(tpos.xzy*3, 11, 0.03, 0.6);
	float baseheight = noise(tpos.xzy*3, 4, 0.004, 0.3);
	baseheight = pow(baseheight, 5)*3;
	height = baseheight*height;
	height*=260;


	tpos =  M * tpos;	
	tpos.y +=height;
	tpos.y -=(1-texheight)*120;


	vertex_pos = tpos.xyz;
	gl_Position = tpos;
	vertex_tex = vertTex;
}
