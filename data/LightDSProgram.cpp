#include "LightDSProgram.hpp"

// SOURCE : http://ogldev.atspace.co.uk/www/tutorial36/tutorial36.html

#include <iostream>

using namespace Vancom;

LightDSProgram::LightDSProgram(){

}

bool LightDSProgram::init(){

    // get uniforms
    locMVP = getUniformLocation("mvp");
    locPosTexture = getUniformLocation("posMap");
    locColorTexture = getUniformLocation("colorMap");
    locNormalTexture = getUniformLocation("normalMap");
    locSpecularTexture = getUniformLocation("specularMap");
    locScreenSize = getUniformLocation("screenSize");
    locCameraPos = getUniformLocation("cameraPos");


    return true;
}

void LightDSProgram::setMVP(const glm::mat4 MVP) const{

    glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(MVP));    
}

void LightDSProgram::setPositionTextureUnit(unsigned int textureUnit){

    glUniform1i(locPosTexture, textureUnit);
}

void LightDSProgram::setColorTextureUnit(unsigned int textureUnit){

    glUniform1i(locColorTexture, textureUnit);
}

void LightDSProgram::setNormalTextureUnit(unsigned int textureUnit){

    glUniform1i(locNormalTexture, textureUnit);
}

void LightDSProgram::setSpecularTextureUnit(unsigned int textureUnit){

    glUniform1i(locSpecularTexture, textureUnit);
}

void LightDSProgram::setCameraPosition(const glm::vec3 pos) const{

    glUniform3fv(locCameraPos, 1, glm::value_ptr(pos));
}

void LightDSProgram::setScreenSize(unsigned int width, unsigned int height){

    glUniform2f(locScreenSize, (float) width, (float) height);
}
