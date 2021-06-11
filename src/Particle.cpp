#include "Particle.h"

float randFloat(float l, float h)
{
    float r = rand() / (float)RAND_MAX;
    return (1.0f - r) * l + r * h;
}

void Particle::load(vec3 pos)
{
    charge = randFloat(0.0f, 1.0f) < 0.5f ? -1.0f : 1.0f;
    damping = randFloat(0.0f, 0.02f);
    position = pos;
    velocity.x = randFloat(-0.2f, 0.2f);
    velocity.y = randFloat(-0.2f, 0.2f);
    velocity.z = randFloat(-0.2f, 0.2f);
//    color = vec4(225.0f/255.0f, 243.0f/255.0f, 166.0f/255.0f, 1.0f);
    color = vec4(1, 0, 0, 1.0f);
}

void Particle::update(float t)
{
    if (color.a > 0.0f) {
        position.x -= velocity.x * t;
        position.z -= velocity.z * t;
        position.y -= velocity.y * t + 0.5 * gravity * t * t;
        color.a -= t * 0.1;
    }

}