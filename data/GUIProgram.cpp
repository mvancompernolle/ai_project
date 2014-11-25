#include "GUIProgram.hpp"

#include <iostream>

using namespace Vancom;

GUIProgram::GUIProgram(){

}

bool GUIProgram::init(){

    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/gui.vs"))
        return false;

	if (!addShader(GL_FRAGMENT_SHADER, "../shaders/gui.fs"))
		return false;

	if (!finalize())
		return false;

    // get uniform and attribute positions in shader
    locSampler = getUniformLocation("textureSampler");

    return true;
}

void GUIProgram::setSampler(const int sampler) const{

    glUniform1i(locSampler, 0);    
}