#define GLEW_STATIC
#define TINYOBJLOADER_IMPLEMENTATION

#include <iostream>
#include <string>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"

#include "Helpers.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "TerrainCollision.hpp"

#include "Model3D.hpp"
#include "Mesh.hpp"
#include "Terrain.hpp"

int glWindowWidth = 1280;
int glWindowHeight = 780;
int retina_width, retina_height;
GLFWwindow *glWindow = NULL;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;

glm::vec3 cameraPosition(-0.815332f, 1.712594f, 6.586054f);
glm::vec3 cameraDir(0.008652f, -0.130526f, -0.991407f);
glm::vec3 cameraTarget = cameraDir + cameraPosition;
gps::Camera camera(cameraPosition, cameraTarget);
float cameraSpeed = 1e-2f;

bool pressedKeys[1024];
float angle = 0.0f;

gps::Model3D* terrainModel;
gps::Model3D* condor;
gps::Model3D* cottage;
std::vector<gps::Model3D*> watchtowers;
std::vector<gps::Model3D*> allModels;

gps::Shader mainProgramShader;
gps::Terrain terrain;
gps::TerrainCollision terrainCollision;

GLenum glCheckError_(const char *file, int line)
{
  GLenum errorCode;
  while ((errorCode = glGetError()) != GL_NO_ERROR)
  {
    std::string error;
    switch (errorCode)
    {
    case GL_INVALID_ENUM:
      error = "INVALID_ENUM";
      break;
    case GL_INVALID_VALUE:
      error = "INVALID_VALUE";
      break;
    case GL_INVALID_OPERATION:
      error = "INVALID_OPERATION";
      break;
    case GL_STACK_OVERFLOW:
      error = "STACK_OVERFLOW";
      break;
    case GL_STACK_UNDERFLOW:
      error = "STACK_UNDERFLOW";
      break;
    case GL_OUT_OF_MEMORY:
      error = "OUT_OF_MEMORY";
      break;
    case GL_INVALID_FRAMEBUFFER_OPERATION:
      error = "INVALID_FRAMEBUFFER_OPERATION";
      break;
    }
    std::cout << error << " | " << file << " (" << line << ")" << std::endl;
  }
  return errorCode;
}

int time = 0;
int dayPeriodLength = 4095;
bool isDay = true;

float projectionAngle = 45.f;

bool inPresentationMode = false;

float ang = 0.0f;

#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow *window, int width, int height)
{
  fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
  //TODO
  //for RETINA display
  glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

  //set projection matrix
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
  //send matrix data to shader
  GLint projLoc = glGetUniformLocation(mainProgramShader.shaderProgram, "projection");
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

  //set Viewport transform
  glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if (key >= 0 && key < 1024)
  {
    if (action == GLFW_PRESS)
      pressedKeys[key] = true;
    else if (action == GLFW_RELEASE)
      pressedKeys[key] = false;
  }
}

void processMovement()
{
  if (pressedKeys[GLFW_KEY_Q])
  {
    angle += 0.1f;
    if (angle > 360.0f)
      angle -= 360.0f;
  }

  if (pressedKeys[GLFW_KEY_E])
  {
    angle -= 0.1f;
    if (angle < 0.0f)
      angle += 360.0f;
  }

  if (pressedKeys[GLFW_KEY_W] && !inPresentationMode)
  {
    camera.move(gps::MOVE_FORWARD, cameraSpeed);
  }

  if (pressedKeys[GLFW_KEY_S] && !inPresentationMode)
  {
    camera.move(gps::MOVE_BACKWARD, cameraSpeed);
  }

  if (pressedKeys[GLFW_KEY_A] && !inPresentationMode)
  {
    camera.move(gps::MOVE_LEFT, cameraSpeed);
  }

  if (pressedKeys[GLFW_KEY_D] && !inPresentationMode)
  {
    camera.move(gps::MOVE_RIGHT, cameraSpeed);
  }

	if (pressedKeys[GLFW_KEY_EQUAL]) {
		// zoom in
		projectionAngle = (float)gps::coerce(projectionAngle - 1e-2, 5.f, 45.f);
		projection = glm::perspective(glm::radians(projectionAngle), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	}

	if (pressedKeys[GLFW_KEY_MINUS]) {
		// zoom out
		projectionAngle = (float)gps::coerce(projectionAngle + 1e-2, 5.f, 45.f);
		projection = glm::perspective(glm::radians(projectionAngle), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	}

	if (pressedKeys[GLFW_KEY_P]) {
		inPresentationMode = false;
	}

	if (pressedKeys[GLFW_KEY_1]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}

	if (pressedKeys[GLFW_KEY_2]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (pressedKeys[GLFW_KEY_3]) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
  static bool firstMouseCallback = true;
  static double xlast, ylast;
  static const float cameraRotationSensitivity = 5e-1;
  static double yaw = 0, pitch = 0;

	if (inPresentationMode) {
		return;
	}

  if (firstMouseCallback) {
    firstMouseCallback = false;
    xlast = xpos;
    ylast = ypos;
    return;
  }

  float offsetx = (xpos - xlast) * cameraRotationSensitivity;
  float offsety = (ypos - ylast) * cameraRotationSensitivity;

  xlast = xpos, ylast = ypos;

  yaw += offsetx;
  pitch += offsety;

  //yaw		= coerce(yaw,		-89.0f, 89.0f);
  pitch = gps::coerce(pitch, -89.0f, 89.0f);
  camera.rotate(-pitch, yaw);
}

bool initOpenGLWindow()
{
  if (!glfwInit())
  {
    fprintf(stderr, "ERROR: could not start GLFW3\n");
    return false;
  }

  // for Mac OS X
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "Marcus Pop project", NULL, NULL);
  if (!glWindow)
  {
    fprintf(stderr, "ERROR: could not open window with GLFW3\n");
    glfwTerminate();
    return false;
  }

  glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
  glfwMakeContextCurrent(glWindow);
  glfwWindowHint(GLFW_SAMPLES, 4);

  // start GLEW extension handler
  glewExperimental = GL_TRUE;
  glewInit();

  // get version info
  const GLubyte *renderer = glGetString(GL_RENDERER); // get renderer string
  const GLubyte *version = glGetString(GL_VERSION);   // version as a string

  printf("Renderer: %s\n", renderer);
  printf("OpenGL version supported %s\n", version);

  // for RETINA display
  glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);
  glfwSetKeyCallback(glWindow, keyboardCallback);

  // disable cursor and bind callback for mouse events
  glfwSetCursorPosCallback(glWindow, mouseCallback);
  glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  return true;
}

void initOpenGLState()
{
  glClearColor(0.3, 0.3, 0.3, 1.0);
  glViewport(0, 0, retina_width, retina_height);

  glEnable(GL_DEPTH_TEST); // enable depth-testing
  glDepthFunc(GL_LESS);    // depth-testing interprets a smaller value as "closer"
  glEnable(GL_CULL_FACE);  // cull face
  glCullFace(GL_BACK);     // cull back face
  glFrontFace(GL_CCW);     // GL_CCW for counter clock-wise
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void initModels()
{
  terrainModel = new gps::Model3D("objects/land/landscape.obj", "objects/land/");
	terrainCollision = gps::TerrainCollision(terrainModel->meshes.front()); // terrain has only one mesh
	camera.setTerrainCollision(terrainCollision);

	watchtowers.push_back(new gps::Model3D("objects/watchtower1/watchtower.obj", "objects/watchtower1/"));
	watchtowers.push_back(new gps::Model3D("objects/watchtower2/watchtower.obj", "objects/watchtower2/"));
	watchtowers.push_back(new gps::Model3D("objects/watchtower3/watchtower.obj", "objects/watchtower3/"));

	condor = new gps::Model3D("objects/condor/Condor/CONDOR.OBJ", "objects/condor/Condor/");
	cottage = new gps::Model3D("objects/cottage/cottage_obj.obj", "objects/cottage/");

	allModels.insert(std::end(allModels), std::begin(watchtowers), std::end(watchtowers));
	allModels.push_back(condor);
	allModels.push_back(cottage);

	camera.setModels(allModels);
}

void initShaders()
{
  mainProgramShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
  mainProgramShader.useShaderProgram();
}

void initUniforms()
{
  model = glm::mat4(1.0f);
  modelLoc = glGetUniformLocation(mainProgramShader.shaderProgram, "model");
  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

  view = camera.getViewMatrix();
  viewLoc = glGetUniformLocation(mainProgramShader.shaderProgram, "view");
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

  normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
  normalMatrixLoc = glGetUniformLocation(mainProgramShader.shaderProgram, "normalMatrix");
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

  projection = glm::perspective(glm::radians(projectionAngle), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
  projectionLoc = glGetUniformLocation(mainProgramShader.shaderProgram, "projection");
  glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

  //set the light direction (direction towards the light)
  lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
  lightDirLoc = glGetUniformLocation(mainProgramShader.shaderProgram, "lightDir");
  glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

  //set light color
  lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
  lightColorLoc = glGetUniformLocation(mainProgramShader.shaderProgram, "lightColor");
  glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
}

void renderScene() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  processMovement();

	if (inPresentationMode) {
		glm::mat4 tr(1.0f);
		tr = glm::rotate(model, glm::radians(ang), glm::vec3(0, 1, 0));

		glm::vec3 newPosition = glm::vec3(tr * glm::vec4(camera.getCameraPosition(), 1.f));
		glm::vec3 newDirection = glm::vec3(tr * glm::vec4(camera.getCameraDirection(), 1.f));

		camera.setCameraPosition(newPosition);
		camera.setCameraDirection(newDirection);
		ang += 1e-4f;

		if (ang > .275f) {
			inPresentationMode = false;
		}
	}

	view = camera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	model = glm::mat4(1.f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

  normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	terrainModel->Draw(mainProgramShader);
	condor->Draw(mainProgramShader);
	cottage->Draw(mainProgramShader);

	for (auto& const watchtower : watchtowers) {
		watchtower->Draw(mainProgramShader);
	}
}

int main(int argc, const char *argv[]) {
  initOpenGLWindow();
  initOpenGLState();
  initModels();
  initShaders();
  initUniforms();

  while (!glfwWindowShouldClose(glWindow))
  {
    renderScene();

    glfwPollEvents();
    glfwSwapBuffers(glWindow);
  }

  //close GL context and any other GLFW resources
  glfwTerminate();

  return 0;
}
