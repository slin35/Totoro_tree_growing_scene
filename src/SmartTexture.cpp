/*
 * SmartTexture.h
 * CSC 474 Cal Poly, Shawn Harris, 2021
 * 
 */
#include "SmartTexture.h"
#include <iostream>
#include <cassert>
#include <fstream>

#define NOMINMAX  
#include <limits>
#include <cstddef>

#include "GLSL.h"

using namespace std;

GLuint SmartTexture::gUnit = 0;

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "stb_image.h"


bool file_exists(const std::string& name)
{
	const ifstream f(name.c_str());
	return f.good();
}

unsigned char *stbiload(const char *filename,int *width,int *height,int *channels, int req_channels)
{
stbi_set_flip_vertically_on_load(true);
return stbi_load(filename, width, height,channels, req_channels);
}


shared_ptr<SmartTexture> SmartTexture::loadTexture(const std::string& filename, bool skyBox)
{
	auto texture = make_shared<SmartTexture>();

	if (texture->init(filename, skyBox))
	{
		gUnit++;
	}
	else
	{
		texture.reset();
	}
	
	return texture;
}


SmartTexture::SmartTexture() : data(NULL), width(0), height(0), tid(0), mUnit(0)
{
}

SmartTexture::~SmartTexture()
{
	if (data)
		stbi_image_free(data);
}

bool SmartTexture::init(const string &filename, bool skyBox)
{
	mUnit = gUnit;
	
	// Load texture
	int w, h, ncomps;

	stbi_set_flip_vertically_on_load(true);
	
	if (!file_exists(filename))
		return false;
	
	data = stbi_load(filename.c_str(), &w, &h, &ncomps, 0);
	if (!data)
		return false;
	
	{
		//cerr << "texture " << actualfilename << " not found" << endl;
		
		//exit(0);
	}
	
	if (ncomps != 3) 
		cerr << "warning: texture " << filename << " must have 3 components (RGB)" << endl;
	
	if((w & (w - 1)) != 0 || (h & (h - 1)) != 0) 
		cerr << "warning: texture " << filename << " must be a power of 2" << endl;
	
	width = w;
	height = h;
	
	// Generate a texture buffer object
	glGenTextures(1, &tid);
	// Bind the current texture to be the newly generated texture object
	glBindTexture(GL_TEXTURE_2D, tid);
	// Load the actual texture data
	// Base level is 0, number of channels is 3, and border is 0.
	if (ncomps == 3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	if (ncomps == 4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		
	if (!skyBox)
	{
		// Generate image pyramid
		glGenerateMipmap(GL_TEXTURE_2D);
		// Set texture wrap modes for the S and T directions
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// Set filtering mode for magnification and minimification
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else
	{
		makeSkybox();
	}
	
	// Unbind
	glBindTexture(GL_TEXTURE_2D, 0);
	// Free image, since the data is now on the GPU
	//stbi_image_free(data);
	return true;
}

void SmartTexture::setWrapModes(GLint wrapS, GLint wrapT)
{
	// Must be called after init()
	glBindTexture(GL_TEXTURE_2D, tid);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
}

void SmartTexture::bind(GLuint handle)
{
	glActiveTexture(GL_TEXTURE0 + mUnit);
	glBindTexture(GL_TEXTURE_2D, tid);
	glUniform1i(handle, mUnit);
}

void SmartTexture::unbind()
{
	glActiveTexture(GL_TEXTURE0 + mUnit);
	glBindTexture(GL_TEXTURE_2D, 0);
}


void SmartTexture::makeSkybox()
{
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
}

	
