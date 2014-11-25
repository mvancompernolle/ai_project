#include "RandomTexture.hpp"

#include <iostream>
#include <random>

using namespace Vancom;

RandomTexture::RandomTexture(){

    textureObj = 0;    
}


RandomTexture::~RandomTexture(){

    if (textureObj != 0) {
        glDeleteTextures(1, &textureObj);
    }
}

void RandomTexture::initRandomTexture(unsigned int size){

    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::vector<glm::vec3> randomData;
    for (unsigned int i = 0 ; i < size ; i++) {
        randomData.push_back(glm::vec3(dist(gen), dist(gen), dist(gen)));
    }
    glGenTextures(1, &textureObj);
    glBindTexture(GL_TEXTURE_1D, textureObj);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, size, 0, GL_RGB, GL_FLOAT, randomData.data());
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);    
}

    
void RandomTexture::bind(GLenum textureUnit){
	
    glActiveTexture(textureUnit);
    glBindTexture(GL_TEXTURE_1D, textureObj);
}
