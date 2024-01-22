#pragma once
#include <stdio.h>
#include <filesystem>
#include <iostream>


float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};


std::vector<std::string> getCubemapFaces() {
	std::vector<std::string> faces;
	faces.push_back("./textures/skybox/sk1/left.png");
    faces.push_back("./textures/skybox/sk1/right.png");
    faces.push_back("./textures/skybox/sk1/top.png");
    faces.push_back("./textures/skybox/sk1/bottom.png");
    faces.push_back("./textures/skybox/sk1/front.png");
    faces.push_back("./textures/skybox/sk1/back.png");
	return faces;
}

std::vector<std::string> getSecondCubemapFaces() {
    std::vector<std::string> faces;
    faces.push_back("./textures/skybox/sk2/left.png");
    faces.push_back("./textures/skybox/sk2/right.png");
    faces.push_back("./textures/skybox/sk2/top.png");
    faces.push_back("./textures/skybox/sk2/bottom.png");
    faces.push_back("./textures/skybox/sk2/front.png");
    faces.push_back("./textures/skybox/sk2/back.png");
    return faces;
}
