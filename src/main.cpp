/*
CPE/CSC 474 Lab base code Eckhardt/Dahl
based on CPE/CSC 471 Lab base code Wood/Dunn/Eckhardt
*/
#define _USE_MATH_DEFINES
#include <iostream>
#include <regex>
#include <fstream>
#include <glad/glad.h>
#include <cmath>



#include "GLSL.h"
#include "Program.h"
#include "WindowManager.h"
#include "Shape.h"
#include "skmesh.h"
#include "stb_image.h"
#include "line.h"
// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// assimp
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "assimp/scene.h"
#include "assimp/vector3.h"
#include "assimp/scene.h"
#include <assimp/mesh.h>

#include "Particle.h"

using namespace std;
using namespace glm;
using namespace Assimp;


#define MESHSIZE 1000
int renderstate = 1;
ofstream file;

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}

class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d, q, e, z, c;
	camera()
	{
		w = a = s = d = q = e = z = c = 0;
		pos = glm::vec3(0, -2, -6);
		rot = glm::vec3(0, 0, 0);
	}

	glm::mat4 process(double ftime)
	{
		float speed = 0;

		float fwdspeed = 10;
		

		if (w == 1)
		{
			speed = fwdspeed * ftime;
		}
		else if (s == 1)
		{
			speed = -fwdspeed * ftime;
		}
		float yangle = 0;
		float ss = 1;
		if (a == 1)
			yangle = -ss * ftime;
		else if (d == 1)
			yangle = ss * ftime;
		rot.y += yangle;
		float zangle = 0;
		if (q == 1)
			zangle = -ss * ftime;
		else if (e == 1)
			zangle = ss * ftime;
		rot.z += zangle;
		float xangle = 0;
		if (z == 1)
			xangle = -ss * ftime;
		else if (c == 1)
			xangle = ss * ftime;
		rot.x += xangle;

		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::mat4 Rz = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));
		glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		R = Rz * Rx * R;
		dir = dir * R;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return R * T;
	}

	void get_dirpos(vec3& up, vec3& dir, vec3& position)
	{
		position = pos;
		glm::mat4 R = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::mat4 Rz = glm::rotate(glm::mat4(1), rot.z, glm::vec3(0, 0, 1));
		glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::vec4 dir4 = glm::vec4(0, 0, 1, 0);
		R = Rz * Rx * R;
		dir4 = dir4 * R;
		dir = vec3(dir4);
		glm::vec4 up4 = glm::vec4(0, 1, 0, 0);
		up4 = R * vec4(0, 1, 0, 0);
		up4 = vec4(0, 1, 0, 0) * R;
		up = vec3(up4);
	}
};

camera mycam;

class Orientation {
public:
	Orientation();
	Orientation(vec3 ez, vec3 ey) : ez(ez), ey(ey) {};

	vec3 ez;
	vec3 ey;
};

class Application : public EventCallbacks
{
public:
	WindowManager * windowManager = nullptr;

	// Our shader program
	std::shared_ptr<Program> psky, skinProg, heightshader, planeprog, linesshader, skyBoxProg, prog, particleprog;

	// Contains vertex information for OpenGL
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint VertexBufferID, VertexNormDBox, VertexTexBox, IndexBufferIDBox;
	GLuint MeshPosID, MeshTexID;

	//texture data
	GLuint Texture;
	GLuint Texture2;
	GLuint HeightTex;
	GLuint terrainTex, fenceTex, houseTex, trunkTex, leafTex, totoroTex, cloudTex, particleTex;

	// skinnedMesh
	SkinnedMesh totoro, cloud, totoro_sleep, tree;

	// textures
	shared_ptr<SmartTexture> skyTex;

	// shapes
	shared_ptr<Shape> skyShape, plane, cube, terrain, fence, house, trunk, fluffytree, leaves;


	// line
	Line linerender;
	Line smoothrender;
	vector<vec3> line;
	vector<vec3> splinepoints;
	vector<Orientation> orientations;

	float gtime = 0;
	bool startSimulation = false;

	// for tree growing simulation
	bool trunkSimulation = false;
	bool branchSimulation = false;
	float trunkPos = -5;
	float branchScale = 0.1;
	float branchSimulationDone = false;
	float leafScale = 3;

	// for totoro scale and pos
	float totoroPos[3] = { 0, 2, 3 };
	float totoroScale[3] = { 1, 0.5, 0.2 };

	bool totoroSimulation = false;

	// for trees
	float tree_angles[30];
	float tree_offsets[30];
	

	// particle system
	std::vector<Particle> particles;
	int numP = 300;
	GLfloat points[900];
	GLfloat pointColors[1200];
	GLuint pointsbuffer;
	GLuint colorbuffer;
	GLuint particleArrayID;
	float lifespan = 1.0f;


	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS)
		{
			mycam.q = 1;
		}
		if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
		{
			mycam.q = 0;
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
			mycam.q = 1;
		}
		if (key == GLFW_KEY_LEFT && action == GLFW_RELEASE) {
			mycam.q = 0;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS)
		{
			mycam.e = 1;
		}
		if (key == GLFW_KEY_E && action == GLFW_RELEASE)
		{
			mycam.e = 0;
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		{
			mycam.e = 1;
		}
		if (key == GLFW_KEY_RIGHT && action == GLFW_RELEASE)
		{
			mycam.e = 0;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS)
		{
			mycam.z = 1;
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE)
		{
			mycam.z = 0;
		}
		if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		{
			mycam.z = 1;
		}
		if (key == GLFW_KEY_UP && action == GLFW_RELEASE)
		{
			mycam.z = 0;
		}
		if (key == GLFW_KEY_C && action == GLFW_PRESS)
		{
			mycam.c = 1;
		}
		if (key == GLFW_KEY_C && action == GLFW_RELEASE)
		{
			mycam.c = 0;
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		{
			mycam.c = 1;
		}
		if (key == GLFW_KEY_DOWN && action == GLFW_RELEASE)
		{
			mycam.c = 0;
		}

		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		{
			mycam.pos = vec3(mycam.pos.x, mycam.pos.y-0.1, mycam.pos.z);
		}

		if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
		{
			vec3 dir, pos, up;
			mycam.get_dirpos(up, dir, pos);
			cout << endl;

			cout << "point position: " << pos.x << ", " << pos.y << ", " << pos.z << endl;
			cout << "Zbase: " << dir.x << ", " << dir.y << ", " << dir.z << endl;
			cout << "Ybase: " << up.x << ", " << up.y << ", " << up.z << endl;
			cout << "point saved into file!" << endl << endl;

			file << "point position: " << pos.x << ", " << pos.y << ", " << pos.z << endl;
			file << "Zbase: " << dir.x << ", " << dir.y << ", " << dir.z << endl;
			file << "Ybase: " << up.x << ", " << up.y << ", " << up.z << endl;
		}


		if (key == GLFW_KEY_F && action == GLFW_PRESS) {
			if (smoothrender.is_active())
				smoothrender.reset();
			else {
				cardinal_curve(splinepoints, line, 10, 5.0);
				smoothrender.re_init_line(splinepoints);
			}

			startSimulation = true;
			gtime = 0;
		}
	}

	// callback for the mouse when clicked move the triangle when helper functions
	// written
	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double posX, posY;
		float newPt[2];
		if (action == GLFW_PRESS)
		{
			glfwGetCursorPos(window, &posX, &posY);
			std::cout << "Pos X " << posX <<  " Pos Y " << posY << std::endl;
		}
	}

	//if the window is resized, capture the new size and reset the viewport
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		//get the window size - may be different then pixels for retina
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}
	/*Note that any gl calls must always happen after a GL state is initialized */
	


	float randFloat(float l, float h)
	{
		float r = rand() / (float)RAND_MAX;
		return (1.0f - r) * l + r * h;
	}
	
	/*Note that any gl calls must always happen after a GL state is initialized */
	void initGeom(const std::string& resourceDirectory)
	{

		for (int i = 0; i < 30; i++) {
			tree_angles[i] = rand() % 180 * 3.14159 / 180;
			tree_offsets[i] = randFloat(-6, 6);
		}

		if (!totoro.LoadMesh(resourceDirectory + "/totoro.fbx", 1)) {
			printf("Mesh load failed\n");
			return;
			}


		if (!totoro_sleep.LoadMesh(resourceDirectory + "/totoro.fbx", 3)) {
			printf("Mesh load failed\n");
			return;
		}


		if (!cloud.LoadMesh(resourceDirectory + "/cloud.fbx")) {
			printf("Mesh load failed\n");
			return;
		}

		if (!tree.LoadMesh(resourceDirectory + "/shakingtree.fbx")) {
			printf("Mesh load failed\n");
			return;
		}

		// Initialize mesh.
		skyShape = make_shared<Shape>();
		skyShape->loadMesh(resourceDirectory + "/sphere.obj");
		skyShape->resize();
		skyShape->init();


		// load terrain
		terrain = make_shared<Shape>();
		terrain->loadMesh(resourceDirectory + "/terrain_big.obj");
		terrain->resize();
		terrain->init();

		
		// load fence
		fence = make_shared<Shape>();
		fence->loadMesh(resourceDirectory + "/fence.obj");
		fence->resize();
		fence->init();
		

		// load house
		house = make_shared<Shape>();
		house->loadMesh(resourceDirectory + "/house.obj");
		house->resize();
		house->init();

		// load trunk
		trunk = make_shared<Shape>();
		trunk->loadMesh(resourceDirectory + "/trunk2.obj");
		trunk->resize();
		trunk->init();

		// load fluffy tree
		fluffytree = make_shared<Shape>();
		fluffytree->loadMesh(resourceDirectory + "/branches2.obj");
		fluffytree->resize();
		fluffytree->init();

		// load leaves
		leaves = make_shared<Shape>();
		leaves->loadMesh(resourceDirectory + "/leaves.obj");
		leaves->resize();
		leaves->init();

		// sky texture
		auto strSky = resourceDirectory + "/FS002_Night.png";
		skyTex = SmartTexture::loadTexture(strSky, true);
		if (!skyTex)
			cerr << "error: texture " << strSky << " not found" << endl;

		
		glGenerateMipmap(GL_TEXTURE_2D);

		// terrain texture
		char filepath[1000];
		int width, height, channels;

		// terrain obj texture
		string str = resourceDirectory + "/grass2.jpg";
		strcpy_s(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &terrainTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, terrainTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);


		// fence texture
		str = resourceDirectory + "/fence.jpg";
		strcpy_s(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &fenceTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, fenceTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		// house texture
		str = resourceDirectory + "/house.png";
		strcpy_s(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &houseTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, houseTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// trunk texture
		str = resourceDirectory + "/trunk_texture.png";
		strcpy_s(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &trunkTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, trunkTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		//  load leaf texture
		str = resourceDirectory + "/leaf.jpg";
		strcpy_s(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &leafTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, leafTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// loud cloud texture
		str = resourceDirectory + "/cloud.jpg";
		strcpy_s(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &cloudTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, cloudTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		// loud cloud texture
		str = resourceDirectory + "/alpha.bmp";
		strcpy_s(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &particleTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, particleTex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		// tree texture
		//texture 1
		str = resourceDirectory + "/tree.PNG";
		strcpy_s(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);


		// initGeom for particles
		CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(14.0f));

		CHECKED_GL_CALL(glGenVertexArrays(1, &particleArrayID));
		CHECKED_GL_CALL(glBindVertexArray(particleArrayID));
		CHECKED_GL_CALL(glGenBuffers(1, &pointsbuffer));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));

		CHECKED_GL_CALL(glGenBuffers(1, &colorbuffer));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));

		initParticles(vec3(0));


		// initilize path1
		smoothrender.init();
		linerender.init();
		parsePath("campath.txt", line, orientations);
		linerender.re_init_line(line);
	}


	float extractFloat(string& w, ifstream& input) {
		regex target("([-]*[0-9]+[.]*[0-9]*[e]*[-]*[0-9]*)");
		smatch match;

		input >> w;
		if (regex_search(w, match, target)) {
			return stof(match.str());
		}

		return -1;
	}

	void parsePath(string filename, vector<vec3>& line, vector<Orientation>& orientations) {
		ifstream file(filename);
		string w;
		vec3 tmp, ez, ey;

		while (file >> w) {
			// get positions
			file >> w;
			tmp.x = extractFloat(w, file);
			tmp.y = extractFloat(w, file);
			tmp.z = extractFloat(w, file);
			line.push_back(tmp * -1.0f);
			// get ez
			file >> w;
			ez.x = extractFloat(w, file);
			ez.y = extractFloat(w, file);
			ez.z = extractFloat(w, file);
			// get ey
			file >> w;
			ey.x = extractFloat(w, file);
			ey.y = extractFloat(w, file);
			ey.z = extractFloat(w, file);
			orientations.push_back(Orientation(ez, ey));
		}
	}


	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		// Enable z-buffer test.
	//	glEnable(GL_DEPTH_TEST);

		// Enable z-buffer test.
		CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(14.0f));

		//glDisable(GL_DEPTH_TEST);
		// Initialize the GLSL program.
		psky = std::make_shared<Program>();
		psky->setVerbose(true);
		psky->setShaderNames(resourceDirectory + "/skyvertex.glsl", resourceDirectory + "/skyfrag.glsl");
		if (!psky->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		
		psky->addUniform("P");
		psky->addUniform("V");
		psky->addUniform("M");
		psky->addUniform("tex");
		psky->addUniform("camPos");
		psky->addAttribute("vertPos");
		psky->addAttribute("vertNor");
		psky->addAttribute("vertTex");

		skinProg = std::make_shared<Program>();
		skinProg->setVerbose(true);
		skinProg->setShaderNames(resourceDirectory + "/skinning_vert.glsl", resourceDirectory + "/skinning_frag.glsl");
		if (!skinProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		
		skinProg->addUniform("P");
		skinProg->addUniform("V");
		skinProg->addUniform("M");
		skinProg->addUniform("tex");
		skinProg->addUniform("camPos");
		skinProg->addAttribute("vertPos");
		skinProg->addAttribute("vertNor");
		skinProg->addAttribute("vertTex");
		skinProg->addAttribute("BoneIDs");
		skinProg->addAttribute("Weights");


		// initialize linesshader
		linesshader = std::make_shared<Program>();
		linesshader->setVerbose(true);
		linesshader->setShaderNames(resourceDirectory + "/lines_height_vertex.glsl", resourceDirectory + "/lines_height_frag.glsl", resourceDirectory + "/lines_geometry.glsl");
		if (!linesshader->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		linesshader->addUniform("P");
		linesshader->addUniform("V");
		linesshader->addUniform("M");
		linesshader->addUniform("camoff");
		linesshader->addUniform("campos");
		linesshader->addAttribute("vertPos");
		linesshader->addAttribute("vertTex");
		linesshader->addUniform("bgcolor");


		// skybox shader
		skyBoxProg = make_shared<Program>();
		skyBoxProg->setVerbose(true);
		skyBoxProg->setShaderNames(resourceDirectory + "/sky_vert.glsl", resourceDirectory + "/sky_frag.glsl");
		if (!skyBoxProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		skyBoxProg->addUniform("P");
		skyBoxProg->addUniform("M");
		skyBoxProg->addUniform("V");
		skyBoxProg->addAttribute("vertPos");
		skyBoxProg->addAttribute("vertNor");


		// normal shader
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/shader_vertex.glsl", resourceDirectory + "/shader_fragment.glsl");
		if (!prog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		prog->addUniform("P");
		prog->addUniform("M");
		prog->addUniform("V");
		prog->addUniform("campos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex");
		prog->addUniform("texoff");
		prog->addUniform("t");
		prog->addUniform("texoff_last");


		particleprog = make_shared<Program>();
		particleprog->setVerbose(true);
		particleprog->setShaderNames(resourceDirectory + "/particle_vert.glsl", resourceDirectory + "/particle_frag.glsl");
		if (!particleprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		particleprog->addUniform("P");
		particleprog->addUniform("M");
		particleprog->addUniform("V");
		particleprog->addUniform("alphaTexture");
		particleprog->addAttribute("vertPos");

	}

	unsigned int createSky(string dir, vector<string> faces) {
		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		int width, height, nrChannels;
		stbi_set_flip_vertically_on_load(false);
		for (GLuint i = 0; i < faces.size(); i++) {
			unsigned char* data =
				stbi_load((dir + faces[i]).c_str(), &width, &height, &nrChannels, 0);
			if (data) {
				glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			}
			else {
				cout << "failed to load: " << (dir + faces[i]).c_str() << endl;
			}
		}
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		cout << " creating cube map any errors : " << glGetError() << endl;
		return textureID;
	}



	mat4 linint_between_two_orientations(vec3 ez_aka_lookto_1, vec3 ey_aka_up_1, vec3 ez_aka_lookto_2, vec3 ey_aka_up_2, float t)
	{
		mat4 m1, m2;
		quat q1, q2;
		vec3 ex, ey, ez;
		ey = ey_aka_up_1;
		ez = ez_aka_lookto_1;
		ex = cross(ey, ez);
		m1[0][0] = ex.x;		m1[0][1] = ex.y;		m1[0][2] = ex.z;		m1[0][3] = 0;
		m1[1][0] = ey.x;		m1[1][1] = ey.y;		m1[1][2] = ey.z;		m1[1][3] = 0;
		m1[2][0] = ez.x;		m1[2][1] = ez.y;		m1[2][2] = ez.z;		m1[2][3] = 0;
		m1[3][0] = 0;			m1[3][1] = 0;			m1[3][2] = 0;			m1[3][3] = 1.0f;
		ey = ey_aka_up_2;
		ez = ez_aka_lookto_2;
		ex = cross(ey, ez);
		m2[0][0] = ex.x;		m2[0][1] = ex.y;		m2[0][2] = ex.z;		m2[0][3] = 0;
		m2[1][0] = ey.x;		m2[1][1] = ey.y;		m2[1][2] = ey.z;		m2[1][3] = 0;
		m2[2][0] = ez.x;		m2[2][1] = ez.y;		m2[2][2] = ez.z;		m2[2][3] = 0;
		m2[3][0] = 0;			m2[3][1] = 0;			m2[3][2] = 0;			m2[3][3] = 1.0f;
		q1 = quat(m1);
		q2 = quat(m2);
		quat qt = slerp(q1, q2, t); //<---
		qt = normalize(qt);
		mat4 mt = mat4(qt);
		//		mt = transpose(mt);		 //<---
		return mt;
	}

	float cosinterp(float t, float f) {
		float cost = 1.0f - (cos(t * M_PI) + 1) / 2;
		return cost * (1 - f) + t * f;
	}

	void initParticles(vec3 pos)
	{
		for (int i = 0; i < numP; ++i)
		{
			auto particle = Particle();
			particles.push_back(particle);
			particle.load(pos);
		}
	}

	void updateGeom()
	{
		glm::vec3 pos;
		glm::vec4 col;

		// go through all the particles and update the CPU buffer
		for (int i = 0; i < numP; i++)
		{
			pos = particles[i].getPosition();
			col = particles[i].getColor();
			points[i * 3 + 0] = pos.x;
			points[i * 3 + 1] = pos.y;
			points[i * 3 + 2] = pos.z;
			pointColors[i * 4 + 0] = col.r + col.a / 10.f;
			pointColors[i * 4 + 1] = col.g + col.g / 10.f;
			pointColors[i * 4 + 2] = col.b + col.b / 10.f;
			pointColors[i * 4 + 3] = col.a;
		}

		// update the GPU data
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 3, points));

		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * numP * 4, pointColors));

		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	void updateParticles()
	{
		// update the particles
		for (auto particle : particles)
		{
			particle.update(0.1);
		}
	}

	void drawParticles(mat4 p, mat4 v, mat4 m) {
		CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(14.0f));

		CHECKED_GL_CALL(glGenVertexArrays(1, &particleArrayID));
		CHECKED_GL_CALL(glBindVertexArray(particleArrayID));
		CHECKED_GL_CALL(glGenBuffers(1, &pointsbuffer));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(points), NULL, GL_STREAM_DRAW));
		CHECKED_GL_CALL(glGenBuffers(1, &colorbuffer));
		CHECKED_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, colorbuffer));
		CHECKED_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(pointColors), NULL, GL_STREAM_DRAW));

		particleprog->bind();
		updateParticles();
		updateGeom();

		//texture6->bind(particleProg->getUniform("alphaTexture"));
		glBindTexture(GL_TEXTURE_2D, particleTex);
		glUniformMatrix4fv(particleprog->getUniform("P"), 1, GL_FALSE, &p[0][0]);
		glUniformMatrix4fv(particleprog->getUniform("V"), 1, GL_FALSE, &v[0][0]);
		glUniformMatrix4fv(particleprog->getUniform("M"), 1, GL_FALSE, &m[0][0]);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, pointsbuffer);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glVertexAttribDivisor(0, 1);
		glVertexAttribDivisor(1, 1);
		// Draw the points !

		glDrawArraysInstanced(GL_POINTS, 0, 1, numP);

		glVertexAttribDivisor(0, 0);
		glVertexAttribDivisor(1, 0);
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glBindBuffer(GL_TEXTURE_2D, 0);


		particleprog->unbind();

		CHECKED_GL_CALL(glDisable(GL_DEPTH_TEST));
		CHECKED_GL_CALL(glDisable(GL_BLEND));

	}

	/****DRAW
	This is the most important function in your program - this is where you
	will actually issue the commands to draw any geometry you have set up to
	draw
	********/

	void render()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		double frametime = get_last_elapsed_time();
		static double totaltime = 0;
		totaltime += frametime;
		gtime += frametime;

		if (gtime > 30) {
			totoroSimulation = true;
		}
		if (gtime > 95) {
			totoro.SetNextAnimation(4);
		}
		else if (gtime > 45) {
			totoro.SetNextAnimation(2);
		}
		if (gtime > 46) {
			trunkSimulation = true;
		}

		// Get current frame buffer size.
		int width, height;
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width / (float)height;
		glViewport(0, 0, width, height);

		// Clear framebuffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Create the matrix stacks - please leave these alone for now

		glm::mat4 V, M, P; //View, Model and Perspective matrix

		mat4 rotCam = mat4(1.0f);
		if (startSimulation) {
			float speed = 3;
			float ftime = gtime * speed;
			int iftime = (int)ftime;
			float t_trans = ftime - iftime;

			if (iftime >= splinepoints.size() - 1) {
				startSimulation = false;
			}
			else {
				vec3 curPos = splinepoints[iftime % splinepoints.size()];
				vec3 nextPos = splinepoints[(iftime + 1) % splinepoints.size()];
				vec3 interpPos = curPos * (1.0f - t_trans) + nextPos * t_trans;

				mycam.pos = interpPos * -1.0f;

				float ftime_o = ftime / (splinepoints.size() / line.size()) * 0.88;
				int iftime_o = (int)ftime_o;
				float t_rot = ftime_o - iftime_o;

				Orientation curDir = orientations[iftime_o % orientations.size()];
				Orientation nextDir = orientations[(iftime_o + 1) % orientations.size()];

				rotCam = transpose(linint_between_two_orientations(curDir.ez, curDir.ey, nextDir.ez, nextDir.ey, t_rot));
			}


		}

		V = rotCam * mycam.process(frametime);

		mat4 Vi = glm::inverse(V);
		M = glm::mat4(1);
		// Apply orthographic projection....
		P = glm::ortho(-1 * aspect, 1 * aspect, -1.0f, 1.0f, -2.0f, 100.0f);
		if (width < height)
		{
			P = glm::ortho(-1.0f, 1.0f, -1.0f / aspect, 1.0f / aspect, -2.0f, 100.0f);
		}
		// ...but we overwrite it (optional) with a perspective projection.
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width / (float)height), 0.1f, 1000.0f); //so much type casting... GLM metods are quite funny ones


		auto sangle = -3.1415926f / 2.0f;
		glm::mat4 RotateXSky = glm::rotate(glm::mat4(1.0f), sangle, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::vec3 camp = -mycam.pos;
		glm::mat4 TransSky = glm::translate(glm::mat4(1.0f), camp);
		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.8f, 0.8f, 0.8f));

		M = TransSky * RotateXSky * SSky;


		glm::mat4 trans, scale;

		// Draw the sky using GLSL.
		psky->bind();
		GLuint texLoc = glGetUniformLocation(psky->pid, "tex");
		skyTex->bind(texLoc);
		glUniformMatrix4fv(psky->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(psky->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(psky->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3fv(psky->getUniform("camPos"), 1, &mycam.pos[0]);

		glDisable(GL_DEPTH_TEST);
		skyShape->draw(psky, false);
		glEnable(GL_DEPTH_TEST);
		skyTex->unbind();
		psky->unbind();


		glDisable(GL_CULL_FACE);


		// draw terrain --------------------------------------------------------------
		prog->bind();
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(100.0f, 100.0f, 100.0f));
		trans = glm::translate(glm::mat4(1.0f), vec3(0, 0, 0));
		M = trans * scale;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, terrainTex);
		terrain->draw(prog, false);
		prog->unbind();

		// draw fence --------------------------------------------------------------
		prog->bind();
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));
		trans = glm::translate(glm::mat4(1.0f), vec3(0, 0.5, 3));
		float angle = -M_PI / 2;
		glm::mat4 rot = glm::rotate(glm::mat4(1.0f), angle, vec3(0, 1, 0));
		M = trans * rot * scale;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, fenceTex);
		fence->draw(prog, false);
		prog->unbind();


		// draw house --------------------------------------------------------------
		prog->bind();
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));
		trans = glm::translate(glm::mat4(1.0f), vec3(0, 2.5, 25));
		rot = glm::rotate(glm::mat4(1.0f), angle, vec3(0, 1, 0));
		M = trans * scale;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, houseTex);
		house->draw(prog, false);
		psky->unbind();


		// draw totoro_sleep --------------------------------------------------------------
		skinProg->bind();
		texLoc = glGetUniformLocation(skinProg->pid, "tex");
		sangle = 3.1415926f / 2.0f;

		glm::mat4 Trans, RotX, Scale;

		Trans = glm::translate(glm::mat4(1.0f), vec3(-3, 0, 23));
		RotX = glm::rotate(glm::mat4(1.0f), sangle, vec3(0, 1, 0));
		Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.001f * 0.5));
		M = Trans * RotX * Scale;

		glUniform3fv(skinProg->getUniform("camPos"), 1, &mycam.pos[0]);
		glUniformMatrix4fv(skinProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(skinProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(skinProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		totoro_sleep.setBoneTransformations(skinProg->pid, frametime * 30);
		totoro_sleep.Render(texLoc);

		skinProg->unbind();


		// simulate trunk growing --------------------------------------------------------------
		prog->bind();
		scale = glm::scale(glm::mat4(1.0f), glm::vec3(5.0f, 5.0f, 5.0f));
		if (trunkSimulation) {
			if (trunkPos < 5) {
				trunkPos += 0.005;
				//	trunkPos += 0.1;
			}
			else {
				branchSimulation = true;
			}
		}
		trans = glm::translate(glm::mat4(1.0f), vec3(0, trunkPos, 3));
		M = trans * scale;
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, trunkTex);
		trunk->draw(prog, false);
		prog->unbind();

		// simulate branch growing --------------------------------------------------------------
		if (branchSimulation) {
			prog->bind();
			if (branchScale < 5) {
				branchScale += 0.01;
			}
			else {
				branchSimulationDone = true;
			}
			scale = glm::scale(glm::mat4(1.0f), glm::vec3(branchScale, branchScale, branchScale));
			trans = glm::translate(glm::mat4(1.0f), vec3(0, trunkPos + (1.0f - branchScale / 5.0f) * 2 + 0.9, 3));
			M = trans * scale;
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, trunkTex);
			fluffytree->draw(prog, false);
			prog->unbind();
		}


		// simulate leaves growing --------------------------------------------------------------
		if (branchSimulationDone) {
			prog->bind();
			if (leafScale < 6) {
				leafScale += 0.1;
			}
			scale = glm::scale(glm::mat4(1.0f), glm::vec3(leafScale));
			trans = glm::translate(glm::mat4(1.0f), vec3(0, 5, 3));
			M = trans * scale;
			glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, leafTex);
			leaves->draw(prog, false);
			prog->unbind();
		}

		// draw totoro --------------------------------------------------------------
		int pos = -10;
		if (totoroSimulation) {
			pos = 0;
		}

		skinProg->bind();
		texLoc = glGetUniformLocation(skinProg->pid, "tex");
		sangle = -3.1415926f / 2.0f;
		sangle = 0.0f;

		for (int i = 0; i < 3; i++) {
			Trans = glm::translate(glm::mat4(1.0f), vec3(totoroPos[i], pos, -3));
			RotX = glm::rotate(glm::mat4(1.0f), sangle, vec3(1, 0, 0));
			Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.001f * totoroScale[i]));
			M = Trans * RotX * Scale;

			glUniform3fv(skinProg->getUniform("camPos"), 1, &mycam.pos[0]);
			glUniformMatrix4fv(skinProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(skinProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(skinProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			totoro.setBoneTransformations(skinProg->pid, frametime * 10);
			totoro.Render(texLoc);
		}

		skinProg->unbind();



		// draw cloud --------------------------------------------------------------
		skinProg->bind();
		texLoc = glGetUniformLocation(skinProg->pid, "tex");
		sangle = 0;

		Trans = glm::translate(glm::mat4(1.0f), vec3(-20, 15, 18));
		RotX = glm::rotate(glm::mat4(1.0f), sangle, vec3(0, 1, 0));
		Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.0001f));
		M = Trans * RotX * Scale;

		glUniform3fv(skinProg->getUniform("camPos"), 1, &mycam.pos[0]);
		glUniformMatrix4fv(skinProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(skinProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(skinProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		cloud.setBoneTransformations(skinProg->pid, frametime * 10);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cloudTex);
		cloud.Render(texLoc);
		skinProg->unbind();


		// draw cloud 2--------------------------------------------------------------
		skinProg->bind();
		texLoc = glGetUniformLocation(skinProg->pid, "tex");
		sangle = -3.1415926f / 2;

		Trans = glm::translate(glm::mat4(1.0f), vec3(0, 10, 30));
		RotX = glm::rotate(glm::mat4(1.0f), sangle, vec3(0, 1, 0));
		Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.0001f));
		M = Trans * RotX * Scale;

		glUniform3fv(skinProg->getUniform("camPos"), 1, &mycam.pos[0]);
		glUniformMatrix4fv(skinProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(skinProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(skinProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		cloud.setBoneTransformations(skinProg->pid, frametime * 10);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cloudTex);
		cloud.Render(texLoc);
		skinProg->unbind();

		// draw cloud 3--------------------------------------------------------------
		skinProg->bind();
		texLoc = glGetUniformLocation(skinProg->pid, "tex");
		sangle = -3.1415926f / 2;

		Trans = glm::translate(glm::mat4(1.0f), vec3(20, 10, 10));
		RotX = glm::rotate(glm::mat4(1.0f), sangle, vec3(0, 1, 0));
		Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.0001f));
		M = Trans * RotX * Scale;

		glUniform3fv(skinProg->getUniform("camPos"), 1, &mycam.pos[0]);
		glUniformMatrix4fv(skinProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(skinProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniformMatrix4fv(skinProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		cloud.setBoneTransformations(skinProg->pid, frametime * 10);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, cloudTex);
		cloud.Render(texLoc);
		skinProg->unbind();


		// load shaking trees ------------------------------------------------------------
		skinProg->bind();
		texLoc = glGetUniformLocation(skinProg->pid, "tex");

		for (int i = -20; i < 20; i += 3) {
			Trans = glm::translate(glm::mat4(1.0f), vec3(-20 + tree_offsets[i], 0, i));
			RotX = glm::rotate(glm::mat4(1.0f), tree_angles[i], vec3(0, 1, 0));
			Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));
			M = Trans * RotX * Scale;

			glUniform3fv(skinProg->getUniform("camPos"), 1, &mycam.pos[0]);
			glUniformMatrix4fv(skinProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(skinProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(skinProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			tree.setBoneTransformations(skinProg->pid, frametime * 0.5);
			tree.Render(texLoc);
		}

		for (int i = -20; i < 20; i += 3) {
			Trans = glm::translate(glm::mat4(1.0f), vec3(20 + tree_offsets[i], 0, i));
			RotX = glm::rotate(glm::mat4(1.0f), tree_angles[i], vec3(0, 1, 0));
			Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.02f));
			M = Trans * RotX * Scale;

			glUniform3fv(skinProg->getUniform("camPos"), 1, &mycam.pos[0]);
			glUniformMatrix4fv(skinProg->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(skinProg->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(skinProg->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			tree.setBoneTransformations(skinProg->pid, frametime * 0.5);
			tree.Render(texLoc);
		}


		skinProg->unbind();


		//draw the dot --------------------------------------------------------------
		//prog->bind();
		//
		//Trans = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
		//Scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f, 0.1f, 0.1f));
		//M = Vi *Trans * Scale;
		//glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		//glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		//glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		//glActiveTexture(GL_TEXTURE0);
		//glBindTexture(GL_TEXTURE_2D, 0);
		//skyShape->draw(prog, false);
		//prog->unbind();

		//// render lines --------------------------------------------------------------
		//vec3 linecolor = glm::vec3(1, 0, 0);
		//linerender.draw(P, V, linecolor);
		//linecolor = glm::vec3(0, 1, 1);
		//smoothrender.draw(P, V, linecolor);
	}

};

//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources"; // Where the resources are loaded from
	std::string missingTexture = "missing.png";
	
	SkinnedMesh::setResourceDir(resourceDir);
	SkinnedMesh::setDefaultTexture(missingTexture);
	
	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	file.open("pathinfo.txt");
	if (!file.is_open())
	{
		cout << "warning! could not open pathinfo.txt file!" << endl;
	}

	Application *application = new Application();

	/* your main will always include a similar set up to establish your window
		and GL context, etc. */
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1920, 1080);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	/* This is the code that will likely change program to program as you
		may need to initialize or set up different data and state */
	// Initialize scene.
	application->init(resourceDir);
	application->initGeom(resourceDir);

	// Loop until the user closes the window.
	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// Render scene.
		application->render();

		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}

