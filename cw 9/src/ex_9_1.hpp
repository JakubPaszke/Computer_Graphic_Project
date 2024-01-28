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

const unsigned int SHADOW_WIDTH = 4024, SHADOW_HEIGHT = 4024;

int WIDTH = 500, HEIGHT = 500;
int isIsland = 1;


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
	Core::RenderContext asteroidContext;
	Core::RenderContext metalContext;
	Core::RenderContext roomContext;
	Core::RenderContext windowContext;

}

namespace texture {
	GLuint defaultTexture;
	GLuint defaultTextureNormal;
	GLuint defaultTextureArm;

	GLuint snowTexture;
	GLuint snowTextureNormal;
	GLuint snowTextureArm;

	GLuint earth;
	GLuint clouds;
	GLuint moon;
	GLuint ship;
	GLuint skybox;
	GLuint grid;
	GLuint sun;
	GLuint metal;
	GLuint asteroid;

	GLuint earthNormal;
	GLuint asteroidNormal;
	GLuint shipNormal;
}

namespace islandPos {
	glm::mat4 lanternPos = glm::mat4() * glm::translate(glm::vec3(-5.73019, 1.0, 0.350166)) * glm::scale(glm::vec3(0.03));
	//glm::mat4 lanternPos = glm::mat4() * glm::translate(glm::vec3(-3.73019, 1.0, -27.350166)) * glm::scale(glm::vec3(0.03));
	glm::mat4 groundPos = glm::mat4() * glm::translate(glm::vec3(0.0f, 1.0f, 0.0f)) * glm::scale(glm::vec3(0.006));
	glm::mat4 portalPos = glm::mat4() * glm::translate(glm::vec3(0.373457, 5.11606, 3.78564)) * glm::scale(glm::vec3(0.3)) * glm::rotate(glm::mat4(1.0f), glm::radians(130.0f), glm::vec3(0.0f, 1.0f, 0.0f)) ;
	glm::mat4 waterPos = glm::mat4()* glm::translate(glm::vec3(20.0f, -6.0f, 20.0f))* glm::scale(glm::vec3(0.7));;
}


namespace planet_pbr {
	struct PlanetTextures {
		GLuint albedo;
		GLuint normal;
		GLuint roughness;
		GLuint metallic;
	};

	PlanetTextures earthTex;
	PlanetTextures mercuryTex;
	PlanetTextures venusTex;
	PlanetTextures marsTex;
	PlanetTextures jupiterTex;
	PlanetTextures saturnTex;
	PlanetTextures uranTex;
	PlanetTextures neptunTex;
	PlanetTextures sunTex;

	PlanetTextures defaultTex;
}



GLuint program;
GLuint program2;
GLuint programSun;
GLuint programTest;
GLuint programTex;
GLuint programDepth;

GLuint programCubemap;
GLuint programIsland;
GLuint skyboxProgram;

Core::Shader_Loader shaderLoader;

Core::RenderContext shipContext;
Core::RenderContext sphereContext;


//zrodlo swiatla ogolnego
glm::vec3 sunPos = glm::vec3(0, 0, 0);
glm::vec3 sunDir = glm::vec3(-10.93633f, 0.351106, 0.003226f);
glm::vec3 sunColor = glm::vec3(0.9f, 0.9f, 0.7f)*500;

//camera
glm::vec3 cameraPos = glm::vec3(0.479490f, 1.250000f, -2.124680f);
glm::vec3 cameraDir = glm::vec3(-0.354510f, 0.000000f, 0.935054f);

//depthmap
GLuint depthMapFBO;
GLuint depthMap;
glm::mat4 lightVP;

//statek
glm::vec3 spaceshipPos = glm::vec3(0.065808f, 1.250000f, -2.189549f);
glm::vec3 spaceshipDir = glm::vec3(-0.490263f, 0.000000f, 0.871578f);
GLuint VAO,VBO;
unsigned int skyboxVAO, skyboxVBO, cubemapTexture, secondCubemapTexture;

float aspectRatio = 1.f;

float exposition = 1.f;

//kula imitujaca slonce
glm::vec3 pointlightPos = glm::vec3(0, 0, 0);
glm::vec3 pointlightColor = sunColor;

glm::vec3 spotlightPos = glm::vec3(0, 0, 0);
glm::vec3 spotlightConeDir = glm::vec3(0, 0, 0);
glm::vec3 spotlightColor = glm::vec3(0.4, 0.4, 0.9)*3;
float spotlightPhi = 3.14 / 4;


std::vector<std::pair<std::string, glm::mat4>> objectData;

float lastTime = -1.f;
float deltaTime = 0.f;
float totalMoney = 0;
float health = 10;
float lastResetedAsteroidTime = glfwGetTime();
float lastSpawnedAsteroidTime = glfwGetTime();
float lastTryMetalSpawnTime = glfwGetTime();

glm::vec3 lightColor = glm::vec3(0.9, 0.7, 0.8) * 100;



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
	float f = 200.;
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

void drawObjectPBR(Core::RenderContext& context, glm::mat4 modelMatrix, planet_pbr::PlanetTextures& planetTextures) {

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform1f(glGetUniformLocation(program, "exposition"), exposition);
	Core::SetActiveTexture(planetTextures.albedo, "albedoMap", program, 0);
	Core::SetActiveTexture(planetTextures.normal, "normalMap", program, 1);
	Core::SetActiveTexture(planetTextures.roughness, "roughnessMap", program, 2);
	Core::SetActiveTexture(planetTextures.metallic, "metallicMap", program, 3);

	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniform3f(glGetUniformLocation(program, "sunDir"), sunPos.x, sunPos.y, sunPos.z);
	glUniform3f(glGetUniformLocation(program, "sunColor"), sunColor.x, sunColor.y, sunColor.z);

	glUniform3f(glGetUniformLocation(program, "lightPos"), pointlightPos.x, pointlightPos.y, pointlightPos.z);
	glUniform3f(glGetUniformLocation(program, "lightColor"), pointlightColor.x, pointlightColor.y, pointlightColor.z);

	glUniform3f(glGetUniformLocation(program, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(program, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(program, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(program, "spotlightPhi"), spotlightPhi);
	Core::DrawContext(context);
}

void drawObjectNoTexturesPBR(Core::RenderContext& context, glm::mat4 modelMatrix, glm::vec3 color, float roughness, float metallic) {

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(programIsland, "depthMap"), 0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniformMatrix4fv(glGetUniformLocation(programIsland, "lightVP"), 1, GL_FALSE, (float*)&lightVP);

	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programIsland, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programIsland, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform1f(glGetUniformLocation(programIsland, "exposition"), exposition);

	glUniform1f(glGetUniformLocation(programIsland, "roughness"), roughness);
	glUniform1f(glGetUniformLocation(programIsland, "metallic"), metallic);

	glUniform3f(glGetUniformLocation(programIsland, "color"), color.x, color.y, color.z);

	glUniform3f(glGetUniformLocation(programIsland, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniform3f(glGetUniformLocation(programIsland, "sunDir"), sunDir.x, sunDir.y, sunDir.z);
	glUniform3f(glGetUniformLocation(programIsland, "sunColor"), sunColor.x, sunColor.y, sunColor.z);

	glUniform3f(glGetUniformLocation(programIsland, "lightPos"), pointlightPos.x, pointlightPos.y, pointlightPos.z);
	glUniform3f(glGetUniformLocation(programIsland, "lightColor"), pointlightColor.x, pointlightColor.y, pointlightColor.z);

	glUniform3f(glGetUniformLocation(programIsland, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(programIsland, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(programIsland, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(programIsland, "spotlightPhi"), spotlightPhi);

	Core::DrawContext(context);

}

void drawObjectPBRIsland(Core::RenderContext& context, glm::mat4 modelMatrix, 
															GLuint texture = texture::defaultTexture, 
															GLuint textureNormal = texture::defaultTextureNormal, 
															GLuint textureArm = texture::defaultTextureArm)
{

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(programTest, "depthMap"), 0);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glUniformMatrix4fv(glGetUniformLocation(programTest, "lightVP"), 1, GL_FALSE, (float*)&lightVP);


	glm::mat4 viewProjectionMatrix = createPerspectiveMatrix() * createCameraMatrix();
	glm::mat4 transformation = viewProjectionMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(programTest, "transformation"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(programTest, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	glUniform1f(glGetUniformLocation(programTest, "exposition"), exposition);
	Core::SetActiveTexture(texture, "texture", programTest, 0);
	Core::SetActiveTexture(textureNormal, "textureNormal", programTest, 1);
	Core::SetActiveTexture(textureArm, "textureArm", programTest, 2);

	glUniform3f(glGetUniformLocation(programTest, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glUniform3f(glGetUniformLocation(programTest, "sunDir"), sunDir.x, sunDir.y, sunDir.z);
	glUniform3f(glGetUniformLocation(programTest, "sunColor"), sunColor.x, sunColor.y, sunColor.z);

	glUniform3f(glGetUniformLocation(programTest, "lightPos"), pointlightPos.x, pointlightPos.y, pointlightPos.z);
	glUniform3f(glGetUniformLocation(programTest, "lightColor"), pointlightColor.x, pointlightColor.y, pointlightColor.z);

	glUniform3f(glGetUniformLocation(programTest, "spotlightConeDir"), spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);
	glUniform3f(glGetUniformLocation(programTest, "spotlightPos"), spotlightPos.x, spotlightPos.y, spotlightPos.z);
	glUniform3f(glGetUniformLocation(programTest, "spotlightColor"), spotlightColor.x, spotlightColor.y, spotlightColor.z);
	glUniform1f(glGetUniformLocation(programTest, "spotlightPhi"), spotlightPhi);
	Core::DrawContext(context);

}

void initDepthMap()
{
	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawObjectDepth(Core::RenderContext& context, glm::mat4 viewProjection, glm::mat4 model) {
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	glUniformMatrix4fv(glGetUniformLocation(programDepth, "viewProjectionMatrix"), 1, GL_FALSE, (float*)&viewProjection);
	glUniformMatrix4fv(glGetUniformLocation(programDepth, "modelMatrix"), 1, GL_FALSE, (float*)&model);
	Core::DrawContext(context);
	//glDisable(GL_CULL_FACE);
}

void renderShadowmapSun() {
	float time = glfwGetTime();
	glUseProgram(programDepth);
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	//uzupelnij o renderowanie glebokosci do tekstury
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	//bindowanie FBO
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//czyszczenie mapy głębokości 
	glClear(GL_DEPTH_BUFFER_BIT);
	//ustawianie programu
	glUseProgram(programDepth);

	//drawObjectDepth(models::roomContext, lightVP, glm::mat4());
	//drawObjectDepth(models::windowContext, lightVP, glm::mat4());

	drawObjectDepth(models::lanternContext, lightVP, islandPos::lanternPos);
	drawObjectDepth(models::portalContext, lightVP, islandPos::portalPos);


	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});


	drawObjectDepth(models::spaceshipContext, lightVP, glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.03f))
	);



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
	planet_pbr::PlanetTextures textureID;
	float moneyToDelete;
	int isDeleted;
	float maxMoney;

	Planet(const std::string& planetName, const glm::mat4& initialModelMatrix, planet_pbr::PlanetTextures planetTexture, float maxMoney, float moneyToDelete)
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
		drawObjectPBR(sphereContext, modelMatrix, textureID);
		objectData.push_back({ name, modelMatrix });
	}

private:
	glm::mat4 calculateModelMatrix(float currentTime) {
		if (name == "earth") {
			return glm::eulerAngleY(currentTime / 15) * glm::translate(glm::vec3(10.f, 0, 0)) * glm::scale(glm::vec3(0.3f));
		}
		else if (name == "venus") {
			return glm::eulerAngleY(currentTime / 50) * glm::translate(glm::vec3(20.f, 0, 0)) * glm::scale(glm::vec3(0.25f));
		}
		else if (name == "mars") {
			return glm::eulerAngleY(currentTime / 8) * glm::translate(glm::vec3(3.f, 0, 0)) * glm::scale(glm::vec3(0.25f));
		}
	}
};

class Asteroid {
public:
	glm::vec3 position;
	glm::mat4 modelMatrix;
	planet_pbr::PlanetTextures textureID;
	glm::vec3 direction;
	float speed;
	float timeSpawned;
	float distanceToPlayer;

	Asteroid() {
		position = generateRandomPosition();
		modelMatrix = glm::translate(position) * glm::scale(glm::vec3(0.0002f));
		textureID = planet_pbr::mercuryTex;
		direction = glm::normalize(spaceshipPos - position);
		speed = 0.03f * deltaTime * 60;
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

		//printf("Distance to player: %f\n", distanceToPlayer);

		if (distanceToPlayer < 0.18f) {
			position = generateRandomPosition();
			timeSpawned = glfwGetTime();
			lastResetedAsteroidTime = timeSpawned;
			health -= 3;
			printf("COLLISION! HEALTH: %f\n", health);
		}
		modelMatrix = glm::translate(position) * glm::scale(glm::vec3(0.00005f));
		direction = glm::normalize(spaceshipPos - position);
		drawObjectPBR(models::asteroidContext, modelMatrix, textureID);
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
	planet_pbr::PlanetTextures textureID;
	glm::vec3 direction;
	float speed;
	float distanceToPlayer;
	int isCollected = 0;

	Metal() {
		position = generateRandomPosition();
		modelMatrix = glm::translate(position) * glm::scale(glm::vec3(0.002f));
		textureID = planet_pbr::mercuryTex;
		direction = glm::normalize(-position);
		speed = 0.002f * deltaTime * 60;
	}

	void draw() {
		if (isCollected == 0) {
			position += direction * speed;
			distanceToPlayer = calculateDistance(spaceshipPos, position);
			if (distanceToPlayer < 0.18f) {
				health += 1;
				printf("METAL CAPTURED! HEALTH: %f\n", health);
				isCollected = 1;
			}
			modelMatrix = glm::translate(position) * glm::scale(glm::vec3(0.0005f));
			drawObjectPBR(models::metalContext, modelMatrix, textureID);

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
	planets.push_back(Planet("earth", glm::eulerAngleY(time / 3) * glm::translate(glm::vec3(5.f, 0, 5.f)) * glm::scale(glm::vec3(0.3f)), planet_pbr::mercuryTex, 1000, 10));
	planets.push_back(Planet("venus", glm::eulerAngleY(time / 10) * glm::translate(glm::vec3(8.f, 0, 8.f)) * glm::scale(glm::vec3(0.25f)), planet_pbr::earthTex, 10000, 100));
	planets.push_back(Planet("mars", glm::eulerAngleY(time / 3/2) * glm::translate(glm::vec3(3.f, 0, 3.f)) * glm::scale(glm::vec3(0.25f)), planet_pbr::mercuryTex, 2000, 2));

	for (int i = 0; i < 3; ++i) {
		asteroids.push_back(Asteroid());
	}
}



void makeLogicOnSpace(GLFWwindow* window) {
	objectData.clear();

	for (Planet& planet : planets) {
		planet.draw();
		planet.update(window);
	}

	if (glfwGetTime() - lastSpawnedAsteroidTime > 10.0f && asteroids.size() < 20) {
		asteroids.push_back(Asteroid());
		lastSpawnedAsteroidTime = glfwGetTime();
	}

	for (Asteroid& asteroid : asteroids) {
		asteroid.draw();
	}

	if (glfwGetTime() - lastTryMetalSpawnTime > 10) {
		if (rand() % 100 >= 97) {
			metals.push_back(Metal());
			printf("Metal spawned!");
		}
		lastTryMetalSpawnTime = glfwGetTime();
	}


	for (Metal& metal : metals) {
		metal.draw();
	}

}

void renderSceneSpace(GLFWwindow* window)	//renderowanie kosmosu
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float time = glfwGetTime();
	updateDeltaTime(time);
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
	

	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});

	drawObjectPBR(shipContext,
		glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.03f)),
		planet_pbr::mercuryTex
	);

	spotlightPos = spaceshipPos + 0.2 * spaceshipDir;
	spotlightConeDir = spaceshipDir;

	// rysowanie i zbieranie z planet
	makeLogicOnSpace(window);

	glUseProgram(0);
	glfwSwapBuffers(window);
}

void renderScenePlanet(GLFWwindow* window)		//renderowanie wyspy
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	float time = glfwGetTime();
	updateDeltaTime(time);
	renderShadowmapSun();
	renderSkybox();	

	
	glUseProgram(programIsland);
	/*drawObjectNoTexturesPBR(models::lanternContext,
		islandPos::lanternPos,
		glm::vec3(0.3f, 0.9f, 0.9f),
		0.5f, 0.3f);*/

	drawObjectNoTexturesPBR(models::groundContext,
		islandPos::groundPos,
		glm::vec3(0.9f, 0.9f, 0.9f),
		0.8f, 0.0f);

	drawObjectNoTexturesPBR(models::portalContext,
		islandPos::portalPos,
		glm::vec3(0.1f, 0.9f, 0.5f),
		0.2f, 0.8f);

	/*drawObjectNoTexturesPBR(models::waterContext,
		islandPos::waterPos,
		glm::vec3(0.1f, 0.3f, 0.8f),
		0.3f, 0.4f);*/
	glUseProgram(0);

	




	//poprawic
	sunColor = sunColor * 3;
	pointlightColor = sunColor;
	sunPos = glm::vec3(-8, 50, -9);
	pointlightPos = sunPos;

	glUseProgram(programTest);
	drawObjectPBRIsland(models::waterContext, islandPos::waterPos, texture::defaultTexture, texture::defaultTextureNormal, texture::defaultTextureArm);
	drawObjectPBRIsland(models::lanternContext, islandPos::lanternPos, texture::snowTexture, texture::snowTextureNormal, texture::snowTextureArm);
	//drawObjectPBRIsland(models::groundContext, islandPos::groundPos, texture::snowTexture, texture::snowTextureNormal, texture::snowTextureArm);

	glUseProgram(0);


	glUseProgram(program);


	glm::vec3 spaceshipSide = glm::normalize(glm::cross(spaceshipDir, glm::vec3(0.f, 1.f, 0.f)));
	glm::vec3 spaceshipUp = glm::normalize(glm::cross(spaceshipSide, spaceshipDir));
	glm::mat4 specshipCameraRotrationMatrix = glm::mat4({
		spaceshipSide.x,spaceshipSide.y,spaceshipSide.z,0,
		spaceshipUp.x,spaceshipUp.y,spaceshipUp.z ,0,
		-spaceshipDir.x,-spaceshipDir.y,-spaceshipDir.z,0,
		0.,0.,0.,1.,
		});

	drawObjectPBR(shipContext,
		glm::translate(spaceshipPos) * specshipCameraRotrationMatrix * glm::eulerAngleY(glm::pi<float>()) * glm::scale(glm::vec3(0.03f)),
		planet_pbr::mercuryTex
	);

	spotlightPos = spaceshipPos + 0.2 * spaceshipDir;
	spotlightConeDir = spaceshipDir;

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
	//program2 = shaderLoader.CreateProgram("shaders/shader_2.vert", "shaders/shader_2.frag");
	programTest = shaderLoader.CreateProgram("shaders/test.vert", "shaders/test.frag");
	programSun = shaderLoader.CreateProgram("shaders/shader_8_sun.vert", "shaders/shader_8_sun.frag");
	programCubemap = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");
	skyboxProgram = shaderLoader.CreateProgram("shaders/shader_skybox.vert", "shaders/shader_skybox.frag");

	programIsland = shaderLoader.CreateProgram("shaders/shader_island.vert", "shaders/shader_island.frag");
	programDepth = shaderLoader.CreateProgram("shaders/shader_depth.vert", "shaders/shader_depth.frag");


	loadModelToContext("./models/sphere.obj", sphereContext);
	loadModelToContext("./models/spaceship.obj", shipContext);

	loadModelToContext("./models/ground.obj", models::groundContext);
	loadModelToContext("./models/portal.obj", models::portalContext);
	loadModelToContext("./models/water.obj", models::waterContext);
	loadModelToContext("./models/lantern.obj", models::lanternContext);

	loadModelToContext("./models/room.obj", models::roomContext);
	loadModelToContext("./models/window.obj", models::windowContext);


	loadModelToContext("./models/asteroid.obj", models::asteroidContext);
	loadModelToContext("./models/metal.obj", models::metalContext);
	
	planet_pbr::mercuryTex.albedo = Core::LoadTexture("textures/mercury_diffuseOriginal.bmp");
	planet_pbr::mercuryTex.normal = Core::LoadTexture("textures/mercury_normal.bmp");
	planet_pbr::mercuryTex.roughness = Core::LoadTexture("textures/mercury_metallic.bmp");
	planet_pbr::mercuryTex.metallic = Core::LoadTexture("textures/mercury_smoothness.bmp");

	planet_pbr::earthTex.albedo = Core::LoadTexture("textures/earth_diffuseOriginal.bmp");
	planet_pbr::earthTex.normal = Core::LoadTexture("textures/earth_normal.bmp");
	planet_pbr::earthTex.roughness = Core::LoadTexture("textures/earth_metallic.bmp");
	planet_pbr::earthTex.metallic = Core::LoadTexture("textures/earth_smoothness.bmp");

	texture::defaultTexture = Core::LoadTexture("textures/default/default.png");
	texture::defaultTextureNormal = Core::LoadTexture("textures/default/default_normalmap.png");
	texture::defaultTextureArm = Core::LoadTexture("textures/default/default_arm.png");

	texture::snowTexture = Core::LoadTexture("textures/snow/snow_diff.jpg");
	texture::snowTextureNormal = Core::LoadTexture("textures/snow/snow_normal.jpg");
	texture::snowTextureArm = Core::LoadTexture("textures/snow/snow_arm.jpg");

	planet_pbr::defaultTex.normal=Core::LoadTexture("textures/default/default.png");
	planet_pbr::defaultTex.albedo = Core::LoadTexture("textures/default/default_arm.png");

	createSkybox();
	initDepthMap();
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

	cameraPos = spaceshipPos - 1.5 * spaceshipDir + glm::vec3(0, 1, 0) * 0.2f;
	cameraDir = spaceshipDir;

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
		exposition -= 0.05;
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
		exposition += 0.05;

	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		printf("spaceshipPos = glm::vec3(%ff, %ff, %ff);\n", spaceshipPos.x, spaceshipPos.y, spaceshipPos.z);
		printf("spaceshipDir = glm::vec3(%ff, %ff, %ff);\n", spaceshipDir.x, spaceshipDir.y, spaceshipDir.z);
		printf("spotlightConeDir = glm::vec3(%ff, %ff, %ff);\n", spotlightConeDir.x, spotlightConeDir.y, spotlightConeDir.z);

	}
	if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) 
	{
		isIsland = 0;
		spaceshipPos = glm::vec3(2, 1, 0);
	}
	if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
	{
		//lecimy na planete, zmieniamy zmienna w celu uzycie 2nd render scene
		isIsland = 1;
		spaceshipPos = glm::vec3(3, 2, 3);
	}

	//cameraDir = glm::normalize(-cameraPos);
}

void renderLoop(GLFWwindow* window) {
	initializePlanetData();
	while (!glfwWindowShouldClose(window))
	{
		processInput(window);
		if (isIsland == 0) 
		{
			

			sunPos = glm::vec3(0, 0, 0);
			sunDir = glm::vec3(-10.93633f, 0.351106, 0.003226f);
			sunColor = glm::vec3(0.9f, 0.9f, 0.7f) * 500;

			pointlightPos = sunPos;
			pointlightColor = sunColor;

			renderSceneSpace(window);

		}
		else 
		{
			//tu jest pojebane w shaderze kuba, wiec pointlight bedace sloncem jest tak naprawde sunPos == pointlightPos itd
			//to mozna uzyc jako lampeczka ale tak to po chuuj to
			

			pointlightPos = glm::vec3(0, 20, 0);
			pointlightColor = glm::vec3(0.9, 0.6, 0.6)*5;


			sunPos = glm::vec3(-4.740971f, 2.149999f, 0.369280f);
			sunDir = glm::vec3(-0.93633f, 0.351106, 0.003226f);
			sunColor = glm::vec3(0.9f, 0.9f, 0.7f) * 5;

			lightVP = glm::ortho(-100.f, 100.f, -10.f, 10.f, -10.0f, 100.0f) * glm::lookAt(sunPos, sunPos - sunDir, glm::vec3(0, 1, 0));
			renderScenePlanet(window);
		}
		glfwPollEvents();
	}
}
//}