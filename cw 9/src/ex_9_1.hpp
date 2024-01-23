#include "glew.h"
#include <GLFW/glfw3.h>
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Texture.h"

#include "Box.cpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include "skybox.h"
#include "SOIL/SOIL.h"
#include "SOIL/stb_image_aug.h"
#include "SOIL/stb_image_aug.h"

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

int WIDTH = 500, HEIGHT = 500;

namespace models {
	
	Core::RenderContext drawerContext;
	Core::RenderContext planeContext;
	Core::RenderContext spaceshipContext;
	Core::RenderContext sphereContext;
	Core::RenderContext testContext;
	Core::RenderContext groundContext;
	Core::RenderContext portalContext;
	Core::RenderContext waterContext;
	Core::RenderContext lanternContext;
}

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

GLuint depthMapFBO;
GLuint depthMap;

GLuint program;
GLuint programSun;
GLuint programTest;
GLuint programTex;

GLuint programCubemap;
GLuint skyboxProgram;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;

//slonce galaktyka
glm::vec3 sunPos = glm::vec3(-4.740971f, 2.149999f, 0.369280f);
glm::vec3 sunDir = glm::vec3(-0.93633f, 0.351106, 0.003226f);
glm::vec3 sunColor = glm::vec3(0.9f, 0.9f, 0.7f)*5;

//slonce wyspa
glm::vec3 secondSunPos = glm::vec3(1000.95, 4.45952, 998.212);
glm::vec3 secondSunColor = glm::vec3(0.8f, 0.6f, 0.2f);

//camera
glm::vec3 cameraPos = glm::vec3(0.479490f, 1.250000f, -2.124680f);
glm::vec3 cameraDir = glm::vec3(-0.354510f, 0.000000f, 0.935054f);

//statek
glm::vec3 spaceshipPos = glm::vec3(0.065808f, 1.250000f, -2.189549f);
glm::vec3 spaceshipDir = glm::vec3(-0.490263f, 0.000000f, 0.871578f);
GLuint VAO,VBO;
unsigned int skyboxVAO, skyboxVBO, cubemapTexture, secondCubemapTexture;

float aspectRatio = 1.f;

float exposition = 1.f;

glm::vec3 pointlightPos = glm::vec3(0, 2, 0);
glm::vec3 pointlightColor = glm::vec3(0.9, 0.6, 0.6);

glm::vec3 spotlightPos = glm::vec3(0, 0, 0);
glm::vec3 spotlightConeDir = glm::vec3(0, 0, 0);
glm::vec3 spotlightColor = glm::vec3(0.4, 0.4, 0.9)*3;
float spotlightPhi = 3.14 / 4;



float lastTime = -1.f;
float deltaTime = 0.f;
int isIsland = 0;

// Planety, Asteroidy i Metale


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
		objectData.push_back({ name, modelMatrix });
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

class Asteroid {
public:
	glm::vec3 position;
	glm::mat4 modelMatrix;
	GLuint textureID;
	glm::vec3 direction;
	float speed;
	float timeSpawned;
	float distanceToPlayer;

	Asteroid() {
		position = generateRandomPosition();
		modelMatrix = glm::translate(position) * glm::scale(glm::vec3(0.0002f));
		textureID = texture::asteroid;
		direction = glm::normalize(spaceshipPos - position);
		speed = 0.002f;
		timeSpawned = glfwGetTime();
	}

	void draw() {
		position += direction * speed;
		if (glfwGetTime() - timeSpawned > 20.0f && glfwGetTime() - lastResetedAsteroidTime > 5.0f) {
			position = generateRandomPosition();
			timeSpawned = glfwGetTime();
			lastResetedAsteroidTime = timeSpawned;
		}
		distanceToPlayer = calculateDistance(spaceshipPos, position);

		if (distanceToPlayer < 0.5f) {
			position = generateRandomPosition();
			timeSpawned = glfwGetTime();
			lastResetedAsteroidTime = timeSpawned;
			health -= 3;
			printf("COLLISION! HEALTH: %f\n", health);
		}
		modelMatrix = glm::translate(position) * glm::scale(glm::vec3(0.0002f));
		direction = glm::normalize(spaceshipPos - position);
		drawObjectTexture(asteroidContext, "asteroid", modelMatrix, textureID);
	}

private:
	glm::vec3 generateRandomPosition() {
		float x = spaceshipPos.y + ((rand() % 100) - 50);
		float z = spaceshipPos.z + ((rand() % 100) - 50);
		return glm::vec3(x, 0, z);
	}
};

class Metal {
public:
	glm::vec3 position;
	glm::mat4 modelMatrix;
	GLuint textureID;
	glm::vec3 direction;
	float speed;
	float distanceToPlayer;
	int isCollected = 0;

	Metal() {
		position = generateRandomPosition();
		modelMatrix = glm::translate(position) * glm::scale(glm::vec3(0.002f));
		textureID = texture::metal;
		direction = glm::normalize(-position);
		speed = 0.001f;
	}

	void draw() {
		if (isCollected == 0) {
			position += direction * speed;
			distanceToPlayer = calculateDistance(spaceshipPos, position);
			if (distanceToPlayer < 0.5f) {
				health += 1;
				printf("METAL CAPTURED! HEALTH: %f\n", health);
				isCollected = 1;
			}
			modelMatrix = glm::translate(position) * glm::scale(glm::vec3(0.002f));
			drawObjectTexture(metalContext, "metal"// + std::to_string(rand() % 1000000)
				, modelMatrix, textureID);

			if (isCollected == 0 && (abs(position.x) > 50 || abs(position.z) > 50)) {
				isCollected = 1;
			}
		}
	}

private:
	glm::vec3 generateRandomPosition() {
		float x = spaceshipPos.y + ((rand() % 100) - 50);
		float z = spaceshipPos.z + ((rand() % 100) - 50);
		return glm::vec3(x, 0, z);
	}
};

std::vector<Asteroid> asteroids;
std::vector<Planet> planets;
std::vector<Metal> metals;

void initializePlanetData() {
	float time = glfwGetTime();
	planets.push_back(Planet("earth", glm::eulerAngleY(time / 30) * glm::translate(glm::vec3(20.f, 0, 0)) * glm::scale(glm::vec3(0.3f)), texture::earth, 1000, 10));
	planets.push_back(Planet("venus", glm::eulerAngleY(time / 100) * glm::translate(glm::vec3(50.f, 0, 0)) * glm::scale(glm::vec3(0.25f)), texture::earth, 10000, 100));
	planets.push_back(Planet("mars", glm::eulerAngleY(time / 15) * glm::translate(glm::vec3(10.f, 0, 0)) * glm::scale(glm::vec3(0.25f)), texture::earth, 2000, 2));

	for (int i = 0; i < 3; ++i) {
		asteroids.push_back(Asteroid());
	}
}

void updateDeltaTime(float time) {
	if (lastTime < 0) {
		lastTime = time;
		return;
	}

	deltaTime = time - lastTime;
	if (deltaTime > 0.1) deltaTime = 0.1;
	lastTime = time;
}
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

void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, float roughness, float metallic) {

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);

	glUniform1f(glGetUniformLocation(program, "roughness"), roughness);
	glUniform1f(glGetUniformLocation(program, "metallic"), metallic);

	glUniform3f(glGetUniformLocation(program, "color"), color.x, color.y, color.z);

	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniform3f(glGetUniformLocation(program, "sunDir"), sunDir.x, sunDir.y, sunDir.z);
	glUniform3f(glGetUniformLocation(program, "sunColor"), sunColor.x, sunColor.y, sunColor.z);

	glUniform3f(glGetUniformLocation(program, "lightPos"), pointlightPos.x, pointlightPos.y, pointlightPos.z);
	glUniform3f(glGetUniformLocation(program, "lightColor"), pointlightColor.x, pointlightColor.y, pointlightColor.z);

	glUniform3f(glGetUniformLocation(program, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(program, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(program, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(program, "spotlightPhi"), spotlightPhi);
	Core::DrawContext(context);

}



void renderShadowapSun() {
	float time = glfwGetTime();
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	//uzupelnij o renderowanie glebokosci do tekstury






	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, WIDTH, HEIGHT);
}


unsigned int createCubemap()
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	std::vector<std::string> faces = getCubemapFaces();
	int width, height, nrChannels;
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
		);
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return textureID;
}
unsigned int createSecondCubemap()
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	std::vector<std::string> faces = getSecondCubemapFaces();
	int width, height, nrChannels;
	for (unsigned int i = 0; i < 6; i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
		);
		stbi_image_free(data);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	return textureID;
}

void createSkybox()
{
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	cubemapTexture = createCubemap();
	secondCubemapTexture = createSecondCubemap();
	glUseProgram(skyboxProgram);
	glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), 0);
}

void renderSkybox() 
{
	glm::mat4 projectionMatrix = createPerspectiveMatrix();
	glm::mat4 viewMatrix = createCameraMatrix();
	glDepthFunc(GL_LEQUAL);
	glUseProgram(skyboxProgram);
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
	glUniformMatrix4fv(glGetUniformLocation(skyboxProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	if (isIsland == 0) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

	}
	else {
		glBindTexture(GL_TEXTURE_CUBE_MAP, secondCubemapTexture);

	}
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);
}

void renderScene(GLFWwindow* window)
{
	//ClearColor(0.4f, 0.4f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float time = glfwGetTime();
	updateDeltaTime(time);
	renderShadowapSun();
	renderSkybox();
	

	//space lamp
	glUseProgram(programSun);
	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * glm::translate(pointlightPos) * glm::scale(glm::vec3(0.7));
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniform3f(glGetUniformLocation(programSun, "color"), sunColor.x / 2, sunColor.y / 2, sunColor.z / 2);
	glUniform1f(glGetUniformLocation(programSun, "exposition"), exposition);
	Core::DrawContext(sphereContext);

	//planety ukladu
	glUseProgram(program);

	drawObjectPBR(sphereContext, 
		glm::translate(pointlightPos) * glm::scale(glm::vec3(0.7)) * glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::scale(glm::vec3(0.3f)), 
		glm::vec3(0.2, 0.7, 0.3), 0.3, 0.0);

	drawObjectPBR(sphereContext,
		glm::translate(pointlightPos) * glm::scale(glm::vec3(0.7)) * glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::eulerAngleY(time) * glm::translate(glm::vec3(1.f, 0, 0)) * glm::scale(glm::vec3(0.1f)),
		glm::vec3(0.5, 0.5, 0.5), 
		0.7, 0.0);

	drawObjectPBR(sphereContext,
		glm::translate(pointlightPos) * glm::scale(glm::vec3(0.9)) * glm::eulerAngleY(time / 5) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::scale(glm::vec3(0.5f)),
		glm::vec3(0.7, 0.7, 0.5),
		0.2, 0.9);

	drawObjectPBR(sphereContext,
		glm::translate(pointlightPos) * glm::scale(glm::vec3(2.0)) * glm::eulerAngleY(time / 7) * glm::translate(glm::vec3(4.f, 0, 0)) * glm::scale(glm::vec3(0.9f)),
		glm::vec3(1.0, 0.0, 0.1),
		0.2, 0.9);


	glUseProgram(programSun);
	glm::mat4 secondViewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 secondSunTransformation = secondViewProjectionMatrix * glm::translate(secondSunPos) * glm::scale(glm::vec3(0.7));
	glUniformMatrix4fv(glGetUniformLocation(programSun, "transformation"), 1, GL_FALSE, (float*)&secondSunTransformation);
	glUniform3f(glGetUniformLocation(programSun, "color"), secondSunColor.x, secondSunColor.y, secondSunColor.z);
	glUniform1f(glGetUniformLocation(programSun, "exposition"), exposition);
	Core::DrawContext(sphereContext);

	//elementy na planecie 1000x1000 coords
	glUseProgram(program);
	drawObjectPBR(models::waterContext,
		glm::mat4() * glm::translate(glm::vec3(1050.0f, -7.0f, 1050.0f)) * glm::scale(glm::vec3(0.7)),
		glm::vec3(0.1, 0.1, 0.9),
		0.4f, 0.2f);

	drawObjectPBR(models::groundContext,
		glm::mat4() * glm::translate(glm::vec3(1000.0f, 0.0f, 1000.0f)) * glm::scale(glm::vec3(0.004)),
		glm::vec3(0.428691f, 0.08022f, 0.036889f),
		0.9f, 0.0f);

	drawObjectPBR(models::portalContext,
		glm::mat4() * glm::translate(glm::vec3(1014.0f, 5.0f, 1006.0f)) * glm::scale(glm::vec3(0.5)) * glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
		glm::vec3(0.8, 0.8, 0.8),
		0.2f, 0.9f);

	drawObjectPBR(models::lanternContext,
			glm::mat4() * glm::translate(glm::vec3(1001.0f, 0.0f, 998.0f)) * glm::scale(glm::vec3(0.05)),
			glm::vec3(0.1, 0.2, 0.8),
			0.2f, 0.9f);

	

	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});


	//drawObjectColor(shipContext,
	//	glm::translate(cameraPos + 1.5 * cameraDir + cameraUp * -0.5f) * inveseCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()),
	//	glm::vec3(0.3, 0.3, 0.5)
	//	);
	drawObjectPBR(shipContext,
		glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.03f)),
		glm::vec3(0.3, 0.8, 0.7),
		0.2,1.0
	);

	spotlightPos = spaceshipPos + 0.2 * spaceshipDir;
	spotlightConeDir = spaceshipDir;



	//test depth buffer
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glUseProgram(programTest);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, depthMap);
	//Core::DrawContext(models::testContext);

	glUseProgram(0);
	glfwSwapBuffers(window);
}
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	aspectRatio = width / float(height);
	glViewport(0, 0, width, height);
	WIDTH = width;
	HEIGHT = height;
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
	program = shaderLoader.CreateProgram("shaders/shader_9_1.vert", "shaders/shader_9_1.frag");
	programTest = shaderLoader.CreateProgram("shaders/test.vert", "shaders/test.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_8_sun.vert", "shaders/shader_8_sun.frag");
	programCubemap = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	skyboxProgram = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");

	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship.obj", shipContext);

	loadModelToContext("./models/ground.obj", models::groundContext);
	loadModelToContext("./models/portal.obj", models::portalContext);
	loadModelToContext("./models/water.obj", models::waterContext);
	loadModelToContext("./models/lantern.obj", models::lanternContext);

	



	createSkybox();
}

void shutdown(GLFWwindow* window)
{
	shaderLoader.DeleteProgram(program);
}

//obsluga wejscia
void processInput(GLFWwindow* window)
{
	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f,1.f,0.f)));
	glm::vec3 spaceshipUp = glm::vec3(0.f, 1.f, 0.f);
	float angleSpeed = 0.05f * deltaTime * 60;
	float moveSpeed = 0.05f * deltaTime * 60;
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

	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		std::cout << "Spaceship Position: (" << spaceshipPos.x << ", " << spaceshipPos.y << ", " << spaceshipPos.z << ")" << std::endl;

	cameraPos = spaceshipPos - 0.5 * spaceshipDir + glm::vec3(0, 1, 0) * 0.2f;
	cameraDir = spaceshipDir;

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		exposition -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		exposition += 0.05;

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		printf("spaceshipPos = glm::vec3(%ff, %ff, %ff);\n", spaceshipPos.x, spaceshipPos.y, spaceshipPos.z);
		printf("spaceshipDir = glm::vec3(%ff, %ff, %ff);\n", spaceshipDir.x, spaceshipDir.y, spaceshipDir.z);
	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
		// Teleport spaceship to the sun
		spaceshipPos = sunPos;
		
		cameraPos = sunPos + glm::vec3(0.0f, 0.5f, 0.0f);
		isIsland = 0;
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
	{
		// Teleport spaceship to a new location
		spaceshipPos = glm::vec3(1000.0f, 3.0f, 1000.0f);
		isIsland = 1;
	}

	//cameraDir = glm::normalize(-cameraPos);

}

// funkcja jest glowna petla
void renderLoop(GLFWwindow* window) {
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		renderScene(window);
		glfwPollEvents();
	}
}
//}