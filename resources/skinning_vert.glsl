#version 330 core
layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec2 vertTex;
layout(location = 2) in vec3 vertNor;
layout(location = 3) in ivec4 BoneIDs;
layout(location = 4) in vec4 Weights;

uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

const int MAX_BONES = 100;
uniform mat4 gBones[MAX_BONES];

out vec3 vertex_pos;
out vec3 vertex_normal;
out vec2 vertex_tex;

void main()
{
	mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
	BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
	BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
	BoneTransform     += gBones[BoneIDs[3]] * Weights[3];

	vec4 tpos =  M * BoneTransform* vec4(vertPos, 1.0);
	vertex_pos = tpos.xyz;
	gl_Position = P * V * tpos;

	vertex_normal = vec4(M *BoneTransform * vec4(vertNor,0.0)).xyz;
	
	vertex_tex = vertTex;
}
