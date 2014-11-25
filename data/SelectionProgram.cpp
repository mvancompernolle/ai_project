#include "SelectionProgram.hpp"

#include <iostream>

using namespace Vancom;

SelectionProgram::SelectionProgram(){

}

bool SelectionProgram::init()
{

    if (!ShaderProgram::init()) {
        return false;
    }

    if (!addShader(GL_VERTEX_SHADER, "../shaders/selection.vs")) {
        return false;
    }

    if (!addShader(GL_FRAGMENT_SHADER, "../shaders/selection.fs")) {
        return false;
    }

    if (!finalize()) {
        return false;
    }

    locMVP = getUniformLocation("mvp");
    locObjectIndex = getUniformLocation("gObjectIndex");

    return true;
}


void SelectionProgram::setMVP(const glm::mat4 mvp) const{

    glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(mvp));      
}


void SelectionProgram::setObjectIndex(const uint objectIndex) const{

    glUniform1ui(locObjectIndex, objectIndex);
}