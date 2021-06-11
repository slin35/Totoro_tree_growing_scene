/*
 * SmartTexture.h
 * CSC 474 Cal Poly, Shawn Harris, 2021
 * 
 */
#pragma once
#ifndef SMART_TEXTURE_H
#define SMART_TEXTURE_H

#include <string>
#include <memory>
#include <glad/glad.h>

class SmartTexture
{
public:
	SmartTexture();
	virtual ~SmartTexture();
	
	bool init(const std::string &f, bool skyBox = false);
	unsigned char *getTextureData() { return data; }
	void bind(GLuint handle);
	void unbind();
	void setWrapModes(GLint wrapS, GLint wrapT); // Must be called after init()
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	
	static std::shared_ptr<SmartTexture> loadTexture(const std::string &f, bool skyBox = false);

protected:
	void makeSkybox();
	
	int width;
	int height;
	GLuint tid;
	GLuint mUnit;
	unsigned char* data;
	
	static GLuint gUnit;
};

unsigned char* stbiload(const char* filename, int* width, int* height, int* channels, int req_channels);

#endif // SMART_TEXTURE_H
