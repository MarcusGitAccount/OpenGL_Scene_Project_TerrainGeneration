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

#include "SkyBox.hpp"
#include "Helpers.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "TerrainCollision.hpp"

#include "Model3D.hpp"
#include "Mesh.hpp"
#include "Terrain.hpp"

float spin = 0;

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
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

GLuint source1Loc, source2Loc;
int source1Weight, source2Weight;

glm::vec3 cameraPosition(-0.815332f, 1.712594f, 6.586054f);
glm::vec3 cameraDir(0.008652f, -0.130526f, -0.991407f);
glm::vec3 cameraTarget = cameraDir + cameraPosition;
gps::Camera camera(cameraPosition, cameraTarget);
float cameraSpeed = 1e-2f;

bool pressedKeys[1024];
float angle = 0.0f;

gps::Model3D* terrainModel;
gps::Model3D *condor, *borb, *dog;
std::vector<gps::Model3D*> watchtowers;
std::vector<gps::Model3D*> allModels;
std::vector<gps::Model3D*> cottages;

gps::Shader mainProgramShader;
gps::Terrain terrain;
gps::TerrainCollision terrainCollision;

glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
GLuint cameraPosLightLoc, cameraDirLightLoc;


int time = 1;
int dayPeriodLength = (1 << 14) - 1;
bool isDay = true;

float projectionAngle = 45.f;
bool inPresentationMode = true;
float ang = 0.0f;

gps::Shader skyboxShader, depthMapShader;
gps::SkyBox daySkybox, nightSkyBox;

GLuint shadowMapFBO;
GLuint depthMapTexture;
const GLuint SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
GLfloat lightAngle;

#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow *window, int width, int height) {
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

void sendLightSourcesWeights() {
	glUniform1f(source1Loc, (float)source1Weight);
	glUniform1f(source2Loc, (float)source2Weight);
	printf("Light weights: %d %d\n", source1Weight, source2Weight);
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

	if (pressedKeys[GLFW_KEY_N]) {
		source1Weight ^= 1;
		pressedKeys[GLFW_KEY_N] = false;
		sendLightSourcesWeights();
	}

	if (pressedKeys[GLFW_KEY_M]) {
		source2Weight ^= 1;
		pressedKeys[GLFW_KEY_M] = false;
		sendLightSourcesWeights();
	}

	if (pressedKeys[GLFW_KEY_C]) {
		camera.setDetectCollision();
		pressedKeys[GLFW_KEY_C] = false;
	}

	if (pressedKeys[GLFW_KEY_Z]) {
		time = 0;
		isDay = !isDay;
		source1Weight = isDay;
		source2Weight = 1;
		sendLightSourcesWeights();
		pressedKeys[GLFW_KEY_Z] = false;
	}

	if (pressedKeys[GLFW_KEY_J]) {

		lightAngle += 0.3f;
		if (lightAngle > 360.0f)
			lightAngle -= 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		mainProgramShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
	}

	if (pressedKeys[GLFW_KEY_L]) {
		lightAngle -= 0.3f;
		if (lightAngle < 0.0f)
			lightAngle += 360.0f;
		glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
		mainProgramShader.useShaderProgram();
		glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
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
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void initModels() {
  terrainModel = new gps::Model3D("objects/land/landscape.obj", "objects/land/");
	terrainCollision = gps::TerrainCollision(terrainModel->meshes.front()); // terrain has only one mesh
	camera.setTerrainCollision(terrainCollision);

	watchtowers.push_back(new gps::Model3D("objects/watchtower1/watchtower.obj", "objects/watchtower1/"));
	watchtowers.push_back(new gps::Model3D("objects/watchtower2/watchtower.obj", "objects/watchtower2/"));
	watchtowers.push_back(new gps::Model3D("objects/watchtower3/watchtower.obj", "objects/watchtower3/"));

	borb = new gps::Model3D("objects/condor/condor.obj", "objects/condor/");
	condor = new gps::Model3D("objects/borb/botb.OBJ", "objects/borb/");
	dog = new gps::Model3D("objects/dog/dog.obj", "objects/dog/");

	cottages.push_back(new gps::Model3D("objects/cottage/cottage.obj", "objects/cottage/"));
	cottages.push_back(new gps::Model3D("objects/cottage2/cottage.obj", "objects/cottage2/"));

	allModels.insert(std::end(allModels), std::begin(watchtowers), std::end(watchtowers));
	allModels.insert(std::end(allModels), std::begin(cottages), std::end(cottages));

	allModels.push_back(condor);
	allModels.push_back(dog);
	allModels.push_back(borb);

	camera.setModels(allModels);
}

void initShaders() {
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");

  mainProgramShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
  mainProgramShader.useShaderProgram();
}

void initUniforms() {
	lightDirMatrixLoc = glGetUniformLocation(mainProgramShader.shaderProgram, "lightDirMatrix");

	source1Weight = 1;
	source1Loc = glGetUniformLocation(mainProgramShader.shaderProgram, "source1Weight");
	glUniform1f(source1Loc, (float)source1Weight);

	source2Weight = 1;
	source2Loc = glGetUniformLocation(mainProgramShader.shaderProgram, "source2Weight");
	glUniform1f(source1Loc, (float)source2Weight);

	cameraPosLightLoc = glGetUniformLocation(mainProgramShader.shaderProgram, "cameraPosLight");
	glUniform3fv(cameraPosLightLoc, GL_FALSE, glm::value_ptr(camera.getCameraPosition()));

	cameraDirLightLoc = glGetUniformLocation(mainProgramShader.shaderProgram, "cameraDirLight");
	glUniform3fv(cameraDirLightLoc, GL_FALSE, glm::value_ptr(camera.getCameraDirection()));

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
	lightColor = glm::vec3(1.f);//glm::vec3(1.0f, 1.0f, 1.0f); //white light
  lightColorLoc = glGetUniformLocation(mainProgramShader.shaderProgram, "lightColor");
  glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

}

void initSkyboxes() {
	std::vector<const GLchar*> daySBfaces;
	std::vector<const GLchar*> nightSBfaces;

	daySBfaces.push_back("skybox/day/right.tga");
	daySBfaces.push_back("skybox/day/left.tga");
	daySBfaces.push_back("skybox/day/top.tga");
	daySBfaces.push_back("skybox/day/bottom.tga");
	daySBfaces.push_back("skybox/day/back.tga");
	daySBfaces.push_back("skybox/day/front.tga");

	nightSBfaces.push_back("skybox/night/right.tga");
	nightSBfaces.push_back("skybox/night/left.tga");
	nightSBfaces.push_back("skybox/night/top.tga");
	nightSBfaces.push_back("skybox/night/bottom.tga");
	nightSBfaces.push_back("skybox/night/back.tga");
	nightSBfaces.push_back("skybox/night/front.tga");

	daySkybox.Load(daySBfaces);
	nightSkyBox.Load(nightSBfaces);

	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(camera.getViewMatrix()));

	projection = glm::perspective(glm::radians(projectionAngle), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBOs() {
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
	const GLfloat near_plane = 1.0f, far_plane = 10.0f;
	glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, camera.getCameraDirection(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void drawAll(gps::Shader shader, bool updateModel = true) {
	dog->Draw(shader);
	terrainModel->Draw(shader);
	for (auto& const watchtower : watchtowers) {
		watchtower->Draw(shader);
	}
	for (auto& const cottage : cottages) {
		cottage->Draw(shader);
	}

	if (true) {
		spin = spin + 1e-1;
		if (spin > 360.f) {
			spin = 0.f;
		}
		model = glm::rotate(glm::mat4(1.f), -glm::radians(spin), glm::vec3(0, 1.f, 0));
		shader.useShaderProgram();
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		borb->Draw(shader);

		borb->boundaries.lo = model * borb->boundaries.lo;
		borb->boundaries.hi = model * borb->boundaries.hi;
	}

}

void renderScene() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  processMovement();

	//render the scene to the depth buffer (first pass
	depthMapShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1, GL_FALSE, glm::value_ptr(model));

	drawAll(depthMapShader, false);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	time = (time + 1) & dayPeriodLength;
	if (time == 0) {
		time = 0;
		isDay = !isDay;
	}

	if (inPresentationMode) {
		glm::mat4 tr(1.0f);
		tr = glm::rotate(tr, glm::radians(ang), glm::vec3(0, 1, 0));

		glm::vec3 newPosition = glm::vec3(tr * glm::vec4(camera.getCameraPosition(), 1.f));
		glm::vec3 newDirection = glm::vec3(tr * glm::vec4(camera.getCameraDirection(), 1.f));

		camera.setCameraPosition(newPosition);
		camera.setCameraDirection(newDirection);
		ang += 1e-4f;

		if (ang > .275f) {
			inPresentationMode = false;
		}
	}

	// second pass
	mainProgramShader.useShaderProgram();
	glUniformMatrix4fv(glGetUniformLocation(mainProgramShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	view = camera.getViewMatrix();
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	mainProgramShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(mainProgramShader.shaderProgram, "shadowMap"), 3);

	model = glm::mat4(1.f);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

  normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
  glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	glUniform3fv(cameraPosLightLoc, GL_FALSE, glm::value_ptr(camera.getCameraPosition()));

	glUniform3fv(cameraDirLightLoc, GL_FALSE, glm::value_ptr(camera.getCameraDirection()));

	for (auto const& model3d : allModels) {
		model3d->boundaries.lo = model * model3d->boundaries.lo;
		model3d->boundaries.hi = model * model3d->boundaries.hi;
	}

	skyboxShader.useShaderProgram();
	if (isDay) {
		daySkybox.Draw(skyboxShader, view, projection);
	} 
	else {
		nightSkyBox.Draw(skyboxShader, view, projection);
	}

	drawAll(mainProgramShader, true);
}

int main(int argc, const char *argv[]) {
  initOpenGLWindow();
  initOpenGLState();
	initSkyboxes();
  initModels();
  initShaders();
  initUniforms();

  while (!glfwWindowShouldClose(glWindow)) {
    renderScene();
    glfwPollEvents();
    glfwSwapBuffers(glWindow);
  }

  glfwTerminate();
  return 0;
}
