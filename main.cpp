/*
* Ocean
*
* Ocean shader with hight map and ray-tracing
*
* 2016 Yuzo(Huang Yuzhong)
*
*/

#include <iostream>
#include <boost/format.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "util.hpp"

#ifdef _WIN32
#include <GL/wglew.h>
#pragma comment(lib, "Opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32.lib")
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#else
#include <GL/glxew.h>
#endif

using namespace std;
using namespace boost;

enum class MouseStatus {
	up,
	down
};

int windowWidth = 800;
int windowHeight = 600;
MouseStatus mouseStatus = MouseStatus::up;
double clickX, clickY;

GLuint canvasProgram, canvasVAO, canvasVBO;
GLint iGlobalTimeLocation, iResolutionLocation, iMouseLocation;

void initProgram();
void initVertex();

void onMouseButton(GLFWwindow* window, int button, int action, int mods);
void onCursorPos(GLFWwindow* window, double xpos, double ypos);
void onResize(GLFWwindow* window, int width, int height);

int main(int argc, char* argv[]) {
	if (!glfwInit()) throw runtime_error("Failed to initialize GLFW");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Ocean", NULL, NULL);
	if (!window) throw runtime_error("Failed to open GLFW window");

	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GL_TRUE);
	glfwSetMouseButtonCallback(window, onMouseButton);
	glfwSetCursorPosCallback(window, onCursorPos);
	glfwSetWindowSizeCallback(window, onResize);
	checkError("glfw init");

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) throw runtime_error("Failed to initialize GLEW");
	checkError("glew init");

#ifdef _WIN32
	wglSwapIntervalEXT(-1);
#else
	glXSwapIntervalEXT(-1);
#endif

	// init program and vertex
	initProgram();
	initVertex();

	// variable used in draw loop
	int count = 0;
	double lastTime = glfwGetTime();
	double nowTime;
	char fpsTitle[32];

	// set render status
	glUseProgram(canvasProgram);
	glBindVertexArray(canvasVAO);

	// draw loop
	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glfwSwapBuffers(window);
		glfwPollEvents();

		// update uniforms
		nowTime = glfwGetTime();
		glUniform1f(iGlobalTimeLocation, nowTime);

		// update fps
		if (++count >= 60) {
			snprintf(fpsTitle, 32, "Ocean - FPS: %.2f", count / (nowTime - lastTime));
			glfwSetWindowTitle(window, fpsTitle);
			lastTime = nowTime;
			count = 0;
		}
	} while (!glfwWindowShouldClose(window));

	glfwTerminate();
	return EXIT_SUCCESS;
}

void initProgram() {
	const char* canvasCode[] = { readFileBytes("shader/canvas.vert"), readFileBytes("shader/canvas.frag") };
	GLenum canvasType[] = { GL_VERTEX_SHADER, GL_FRAGMENT_SHADER };
	canvasProgram = buildProgram(canvasCode, canvasType, 2);

	glUseProgram(canvasProgram);
	iGlobalTimeLocation = glGetUniformLocation(canvasProgram, "iGlobalTime");
	iResolutionLocation = glGetUniformLocation(canvasProgram, "iResolution");
	iMouseLocation = glGetUniformLocation(canvasProgram, "iMouse");

	glUniform3f(iResolutionLocation, windowWidth, windowHeight, 0);
	glUseProgram(0);

	delete[] canvasCode[0];
	delete[] canvasCode[1];
	checkError("initProgram");
}

void initVertex() {
	GLfloat canvasVertex[] = {
		-1.0f, -1.0f,
		-1.0f,  1.0f,
		1.0f, -1.0f,
		1.0f,  1.0f
	};

	glGenVertexArrays(1, &canvasVAO);
	glBindVertexArray(canvasVAO);
	glGenBuffers(1, &canvasVBO);
	glBindBuffer(GL_ARRAY_BUFFER, canvasVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(canvasVertex), canvasVertex, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void onMouseButton(GLFWwindow* window, int button, int action, int mods) {
	switch (action) {
		case GLFW_PRESS:
			mouseStatus = MouseStatus::down;
			glfwGetCursorPos(window, &clickX, &clickY);
			break;
		case GLFW_RELEASE:
			mouseStatus = MouseStatus::up;
			break;
	}
}

void onCursorPos(GLFWwindow* window, double xpos, double ypos) {
	if (mouseStatus == MouseStatus::down) {
		glUniform4f(iMouseLocation, xpos, ypos, clickX, clickY);
	}
}

void onResize(GLFWwindow* window, int width, int height) {
	windowWidth = width;
	windowHeight = height;
	glUniform3f(iResolutionLocation, width, height, 0);
	glViewport(0, 0, width, height);
	checkError("onResize");
}