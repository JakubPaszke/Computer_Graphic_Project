#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include <vector>
#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"

#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include "SOIL/SOIL.h"


namespace texture {
	GLuint earth;
	GLuint clouds;
	GLuint moon;
	GLuint ship;
	GLuint skybox;
	GLuint grid;
	GLuint sun;

	GLuint earthNormal;
	GLuint asteroidNormal;
	GLuint shipNormal;
}


GLuint program;
GLuint programSun;
GLuint programTex;
GLuint programEarth;
GLuint programProcTex;
GLuint programCubemap;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;
Core::RenderContext cubemapContext;

glm::vec3 cameraPos = glm::vec3(-4.f, 0, 0);
glm::vec3 cameraDir = glm::vec3(1.f, 0.f, 0.f);
glm::vec3 lightColor = glm::vec3(0.9, 0.7, 0.8) * 100;

glm::vec3 spaceshipPos = glm::vec3(-4.f, 0, 0);
glm::vec3 spaceshipDir = glm::vec3(1.f, 0.f, 0.f);
glm::vec3 sunPos = glm::vec3();
glm::vec3 earthPos = glm::vec3();
glm::vec3 moonPos = glm::vec3();


GLuint VAO,VBO;

float aspectRatio = 1.f;
float totalMoney = 0;

std::vector<std::pair<std::string, glm::mat4>> objectData;


glm::mat4 createCameraMatrix()
{
	glm::vec3 cameraSide = glm::normalize(glm::cross(cameraDir,glm::vec3(0.f,1.f,0.f)));
	glm::vec3 cameraUp = glm::normalize(glm::cross(cameraSide,cameraDir));
	glm::mat4 cameraRotrationMatrix = glm::mat4({
		cameraSide.x,cameraSide.y,cameraSide.z,0,
		cameraUp.x,cameraUp.y,cameraUp.z ,0,
		-cameraDir.x,-cameraDir.y,-cameraDir.z,0,
		0.,0.,0.,1.,
		});
	cameraRotrationMatrix = glm::transpose(cameraRotrationMatrix);
	glm::mat4 cameraMatrix = cameraRotrationMatrix * glm::translate(-cameraPos);

	return cameraMatrix;
}

glm::mat4 createPerspectiveMatrix()
{
	
	glm::mat4 perspectiveMatrix;
	float n = 0.05;
	float f = 20.;
	float a1 = glm::min(aspectRatio, 1.f);
	float a2 = glm::min(1 / aspectRatio, 1.f);
	perspectiveMatrix = glm::mat4({
		1,0.,0.,0.,
		0.,aspectRatio,0.,0.,
		0.,0.,(f+n) / (n - f),2*f * n / (n - f),
		0.,0.,-1.,0.,
		});

	
	perspectiveMatrix=glm::transpose(perspectiveMatrix);

	return perspectiveMatrix;
}

void drawObjectTexture(Core::RenderContext& context, const std::string& objectName, glm::mat4 modelMatrix, GLuint textureID) {
	glUseProgram(programTex);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programTex, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programTex, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniform3f(glGetUniformLocation(programTex, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(programTex, "lightPos"), 0, 0, 0);
	glUniform3f(glGetUniformLocation(program, "lightColor"), lightColor.x, lightColor.y, lightColor.z);
	Core::SetActiveTexture(textureID, "colorTexture", programTex, 0);
	Core::DrawContext(context);
}

void drawCubemap(Core::RenderContext& context, GLuint textureID) {
	glm::mat4 modelMatrix = glm::translate(glm::mat4() * glm::scale(glm::vec3(3.0f)), cameraPos);
	glUseProgram(programCubemap);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programCubemap, "transformation"), 1, GL_FALSE, (float*)&transformation);
	Core::SetActiveTexture(textureID, "skybox", programCubemap, 0);
	Core::DrawContext(context);
}
	


GLuint loadCubeMap(const char* filepaths[6])
{
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int w, h;
	unsigned char* data;

	for (unsigned int i = 0; i < 6; i++)
	{
		const char* filepath = filepaths[i];
		unsigned char* image = SOIL_load_image(filepath, &w, &h, 0, SOIL_LOAD_RGBA);

		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image
		);

		SOIL_free_image_data(image);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

glm::vec3 extractPosition(const std::string& objectName, const std::vector<std::pair<std::string, glm::mat4>>& objectMatrices) {
	for (const auto& entry : objectMatrices) {
		if (entry.first == objectName) {
			return glm::vec3(entry.second[3]);
		}
	}

	return glm::vec3(0.0f);
}


float calculateDistance(glm::vec3 point1, glm::vec3 point2) {
	return glm::length(point1 - point2);
}


class Planet {
public:
	std::string name;
	float timeDelta;
	glm::mat4 modelMatrix;
	GLuint textureID;
	float moneyToDelete;
	int isDeleted;
	float maxMoney;

	Planet(const std::string& planetName, const glm::mat4& initialModelMatrix, GLuint planetTexture, float maxMoney, float moneyToDelete)
		: name(planetName), timeDelta(0.0f), modelMatrix(initialModelMatrix), textureID(planetTexture), maxMoney(maxMoney), moneyToDelete(moneyToDelete), isDeleted(0) {}

	void update(GLFWwindow* window) {
		if (isDeleted == 1) {
			return;
		}
		float time = glfwGetTime();
		float distanceToPlayer = calculateDistance(spaceshipPos, extractPosition(name, objectData));
		const float collectionRadius = 1.0f;
		const float moneyToCollect = glm::min((time - timeDelta) / 100, maxMoney);

		//printf("Name: %s \n distance to player: %f\n spaceshipPos: %f %f %f\n position: %f %f %f\n", name.c_str(), distanceToPlayer, spaceshipPos.x, spaceshipPos.y, spaceshipPos.z, extractPosition(name, objectData).x, extractPosition(name, objectData).y, extractPosition(name, objectData).z);

		if (distanceToPlayer < collectionRadius) {
			if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
				totalMoney += moneyToCollect;
				printf("deltaTime - time: %f, ", time - timeDelta);
				timeDelta = time;
				printf("collected money for %s: %f\n", name.c_str(), moneyToCollect);
				printf("total money %f\n", totalMoney);
				glfwWaitEventsTimeout(0.1f);
			}
			else if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
				if (totalMoney > moneyToDelete) {
					totalMoney -= moneyToDelete;
					printf("The planet was destroyed. Cost: %f, current money: %f\n", moneyToDelete, totalMoney);
					isDeleted = 1;
					glfwWaitEventsTimeout(0.1f);
				}
				else {
					printf("Not enaugh money. Needed: %f, You have: %f\n", moneyToDelete, totalMoney);
					glfwWaitEventsTimeout(0.1f);
				}
			}
		}
	}

	void draw() {
		if (isDeleted == 1) {
			return;
		}
		float time = glfwGetTime();
		modelMatrix = calculateModelMatrix(time);
		drawObjectTexture(sphereContext, name, modelMatrix, texture::earth);
		objectData.push_back({ name, modelMatrix});
	}

private:
	glm::mat4 calculateModelMatrix(float currentTime) {
		if (name == "earth") {
			return glm::eulerAngleY(currentTime / 30) * glm::translate(glm::vec3(20.f, 0, 0)) * glm::scale(glm::vec3(0.3f));
		}
		else if (name == "venus") {
			return glm::eulerAngleY(currentTime / 100) * glm::translate(glm::vec3(50.f, 0, 0)) * glm::scale(glm::vec3(0.25f));
		}
		else if (name == "mars") {
			return glm::eulerAngleY(currentTime / 15) * glm::translate(glm::vec3(10.f, 0, 0)) * glm::scale(glm::vec3(0.25f));
		}
	}
};


std::vector<Planet> planets;

void initializePlanetData() {
	float time = glfwGetTime();
	planets.push_back(Planet("earth", glm::eulerAngleY(time / 30) * glm::translate(glm::vec3(20.f, 0, 0)) * glm::scale(glm::vec3(0.3f)), texture::earth, 1000, 10));
	planets.push_back(Planet("venus", glm::eulerAngleY(time / 100) * glm::translate(glm::vec3(50.f, 0, 0)) * glm::scale(glm::vec3(0.25f)), texture::earth, 10000, 100));
	planets.push_back(Planet("mars", glm::eulerAngleY(time / 15) * glm::translate(glm::vec3(10.f, 0, 0)) * glm::scale(glm::vec3(0.25f)), texture::earth, 2000, 2));
}


void renderScene(GLFWwindow* window)
{
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 transformation;
	float time = glfwGetTime();

	/*drawCubemap(cubemapContext, texture::skybox);*/




	objectData.clear();

	drawObjectTexture(sphereContext, "sun", glm::mat4() * glm::scale(glm::vec3(1.0f)), texture::sun);
	objectData.push_back({ "sun", glm::mat4() });

	// statek
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});

	drawObjectTexture(shipContext, "Ship",
		glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()),
		texture::ship
	);

	// rysowanie i zbieranie z planet
	for (Planet& planet : planets) {
		planet.draw();
		planet.update(window);
	}

	glUseProgram(0);
	glfwSwapBuffers(window);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
}
void loadModelToContext(std::string path, Core::RenderContext& context)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_CalcTangentSpace);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	context.initFromAssimpMesh(scene->mMeshes[0]);
}

void init(GLFWwindow* window)
{
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glEnable(GL_DEPTH_TEST);
	program = shaderLoader.CreateProgram("shaders/shader_5_1.vert", "shaders/shader_5_1.frag");
	programTex = shaderLoader.CreateProgram("shaders/shader_5_1_tex.vert", "shaders/shader_5_1_tex.frag");
	programEarth = shaderLoader.CreateProgram("shaders/shader_5_1_tex.vert", "shaders/shader_5_1_tex.frag");
	programProcTex = shaderLoader.CreateProgram("shaders/shader_5_1_tex.vert", "shaders/shader_5_1_tex.frag");
	programCubemap = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");


	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship.obj", shipContext);
	loadModelToContext("./models/cube.obj", cubemapContext);


	const char* skyboxFilepaths[6] = {
		"textures/skybox/space_bk.png",
		"textures/skybox/space_dn.png",
		"textures/skybox/space_ft.png",
		"textures/skybox/space_lf.png",
		"textures/skybox/space_rt.png",
		"textures/skybox/space_up.png"
	};


	texture::skybox = loadCubeMap(skyboxFilepaths);

	texture::earth = Core::LoadTexture("textures/earth.png");
	texture::ship = Core::LoadTexture("textures/spaceship.jpg");
	texture::sun = Core::LoadTexture("textures/sun.jpg");
	texture::moon = Core::LoadTexture("textures/moon.jpg");


}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
}

//obsluga wejscia
void processInput(GLFWwindow* window)
{
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.005f;
	float moveSpeed = 0.005f;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		spaceshipPos += spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		spaceshipPos -= spaceshipDir * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		spaceshipPos += spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		spaceshipPos -= spaceshipSide * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		spaceshipPos += spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		spaceshipPos -= spaceshipUp * moveSpeed;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(angleSpeed) * glm::vec4(spaceshipDir, 0));
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		spaceshipDir = glm::vec3(glm::eulerAngleY(-angleSpeed) * glm::vec4(spaceshipDir, 0));

	cameraPos = spaceshipPos - 1.5 * spaceshipDir + glm::vec3(0, 1, 0) * 0.5f;
	cameraDir = spaceshipDir;

	//cameraDir = glm::normalize(-cameraPos);

}

// funkcja jest glowna petla
void renderLoop(GLFWwindow* window) {
	initializePlanetData();
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderScene(window);
		glfwPollEvents();
	}
}
//}