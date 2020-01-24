//#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shader_s.h"
#include "stb_image.h"
#include "fps_camera.h"
#include "erosion.h"
#include "TrianglePlane.h"

float Width = 800;
float Height = 600;
glm::mat4 projection = glm::perspective(glm::radians(45.0f), Width / Height, 0.1f, 600.0f);;

class Timing
{
	float lastFrame = 0.0f;

public:
	float deltaTime = 0.0f;

	void Update()
	{
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
	}
};

Timing timer;

bool key_w = false;
bool key_s = false;
bool key_a = false;
bool key_d = false;
bool key_e = false;
bool key_q = false;

Camera camera(glm::vec3(-125.f, 160.f, 100.f), glm::vec3(0, 1, 0),0,-32);
float lastX = Width / 2.0f;
float lastY = Height / 2.0f;
bool firstMouse = true;


void CursorCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	Width = width;
	Height = height;
	projection = glm::perspective(glm::radians(45.0f), Width / Height, 0.1f, 600.0f);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		key_s = true;
	}
	else if (key == GLFW_KEY_S && action == GLFW_RELEASE)
	{
		key_s = false;
	}
	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		key_w = true;
	}
	else if (key == GLFW_KEY_W && action == GLFW_RELEASE)
	{
		key_w = false;
	}
	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		key_a = true;
	}
	else if (key == GLFW_KEY_A && action == GLFW_RELEASE)
	{
		key_a = false;
	}
	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		key_d = true;
	}
	else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
	{
		key_d = false;
	}
	if (key == GLFW_KEY_E && action == GLFW_PRESS)
	{
		key_e = true;
	}
	else if (key == GLFW_KEY_E && action == GLFW_RELEASE)
	{
		key_e = false;
	}
	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		key_q = true;
	}
	else if (key == GLFW_KEY_Q && action == GLFW_RELEASE)
	{
		key_q = false;
	}
}

GLFWwindow* CreateWindow(int Width, int Height, const char* title)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(Width, Height, title, NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetCursorPosCallback(window, CursorCallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

	int nrAttributes;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
	std::cout << "Maximum number of vertex attributes supported: " << nrAttributes << std::endl;

	return window;
}

void KeyEvents()
{
	if (key_w)
		camera.ProcessKeyboard(FORWARD, timer.deltaTime);
	if (key_s)
		camera.ProcessKeyboard(BACKWARD, timer.deltaTime);
	if (key_q)
		camera.ProcessKeyboard(UP, timer.deltaTime);
	if (key_e)
		camera.ProcessKeyboard(DOWN, timer.deltaTime);
	if (key_a)
		camera.ProcessKeyboard(LEFT, timer.deltaTime);
	if (key_d)
		camera.ProcessKeyboard(RIGHT, timer.deltaTime);
}

void UpdateTerrain(Erosion e, TrianglePlane *t)
{
	for (int i = 0; i < t->size; i++)
	{
		t->SetHeight(i, (e.grid->arr[i]) * 100.0);
	}
	t->UpdateBuffer();
}

int main()
{
	GLFWwindow* window = CreateWindow(Width, Height, "erosion!");
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	Shader ourShader("v.glsl", "f.glsl");

	ourShader.use();

	Erosion::Config c;
	Erosion erosion(201,201,600000, c);
	erosion.grid->Blur(1.0 / 3.0);
	//TrianglePlane t(200, 200, &erosion.grid->arr[0]);
	TrianglePlane *t = new TrianglePlane(200, 200);

	UpdateTerrain(erosion, t);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0,0,0));

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);

	while (!glfwWindowShouldClose(window))
	{
		timer.Update();
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		KeyEvents();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS)
			glfwMaximizeWindow(window);

		ourShader.setMat4("model", model);
		ourShader.setMat4("view", camera.GetViewMatrix());
		ourShader.setMat4("projection", projection);
		ourShader.use();

		/*erosion.Erode(10);
		UpdateTerrain(erosion, t);*/
		t->Draw();
		

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}