


#pragma once

#ifndef LAB474_LINE_H_INCLUDED
#define LAB474_LINE_H_INCLUDED

#include <string>
#include <vector>
#include <memory>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
using namespace std;



class Program;

class Line
{

public:
	//stbi_load(char const *filename, int *x, int *y, int *comp, int req_comp)
	bool init();
	bool re_init_line(std::vector<vec3> &points);
	void draw(mat4 &P, mat4 &V, vec3 &colorvec3);
	bool is_active();
	void reset();

private:
	size_t segment_count = 0;
	unsigned int posBufID = 0;	
	unsigned int vaoID;
	unsigned int pid;
	unsigned int ucolor,uP,uV;
};
void cardinal_curve(vector<vec3> &result_path, vector<vec3> &original_path, int lod, float curly);

#endif // LAB471_SHAPE_H_INCLUDED
