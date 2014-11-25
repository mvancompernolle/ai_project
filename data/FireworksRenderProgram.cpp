#include "FireworksRenderProgram.hpp"

using namespace Vancom;

FireworksRenderProgram::FireworksRenderProgram(){

}

bool FireworksRenderProgram::init(){
    
    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/fireworksRender.vs"))
        return false;

    if (!addShader(GL_GEOMETRY_SHADER, "../shaders/fireworksRender.gs"))
        return false;

	if (!addShader(GL_FRAGMENT_SHADER, "../shaders/fireworksRender.fs"))
		return false;

	if (!finalize())
		return false;

    locMVP = getUniformLocation("mvp");
    locCameraPos = getUniformLocation("cameraPos");
    locColorMap = getUniformLocation("colorMap");
    locSize = getUniformLocation("size");

    return true;
}

void FireworksRenderProgram::setMVP(const glm::mat4 MVP){
    glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(MVP));    
}

void FireworksRenderProgram::setCameraPos(const glm::vec3& pos){
    glUniform3f(locCameraPos, pos.x, pos.y, pos.z);
}

void FireworksRenderProgram::setColorTextureUnit(unsigned int textureUnit){
    glUniform1i(locColorMap, textureUnit);
}

void FireworksRenderProgram::setBillboardSize(float billboardSize){
    glUniform1f(locSize, billboardSize);
}