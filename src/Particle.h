#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include <vector>
#include <memory>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "Particle.h"
#include "GLSL.h"
#include "Program.h"

using namespace glm;
using namespace std;


class Particle {

public:

	void update(float t);
	void load(vec3 pos);

	const vec3& getPosition() const { return position; };
	const vec3& getVelocity() const { return velocity; };
	const vec4& getColor() const { return color; };

	vec3 position;
	vec4 color = vec4(1.0f, 0.0f, 0.0f, 1.0f);  //bloody color

private:

	float charge = 1.f; // +1 or -1
	float mass = 1.f;
	float damping;
	vec3 velocity;

	float gravity = 0.1;

};

#endif