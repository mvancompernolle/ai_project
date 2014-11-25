#include "DefaultProgram.hpp"

#include <iostream>

using namespace Vancom;

DefaultProgram::DefaultProgram(){

}

bool DefaultProgram::init(){

    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/default.vs"))
        return false;

	if (!addShader(GL_FRAGMENT_SHADER, "../shaders/default.fs"))
		return false;

	if (!finalize())
		return false;

    // get uniform and attribute positions in shader
    locMVP = getUniformLocation("mvp");

    locPos = glGetAttribLocation(program, "vs_pos");
    locColor = glGetAttribLocation(program, "vs_color");

    if(locMVP == -1 || locPos == -1 || locColor == -1)
        std::cerr << "Unable to find location in DefaultProgram" << std::endl;

    return true;
}

void DefaultProgram::setMVP(const glm::mat4 MVP){

    glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(MVP));    
}
