#include "StencilProgram.hpp"

#include <iostream>

using namespace Vancom;

StencilProgram::StencilProgram(){

}

bool StencilProgram::init(){

    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/stencil.vs"))
        return false;

	if (!addShader(GL_FRAGMENT_SHADER, "../shaders/stencil.fs"))
		return false;

	if (!finalize())
		return false;

    // get uniform and attribute positions in shader
    locMVP = getUniformLocation("mvp");

    return true;
}

void StencilProgram::setMVP(const glm::mat4 MVP){

    glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(MVP));    
}
