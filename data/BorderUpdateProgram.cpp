#include "BorderUpdateProgram.hpp"

#include <iostream>

using namespace Vancom;

BorderUpdateProgram::BorderUpdateProgram(){

}

bool BorderUpdateProgram::init(){

    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/borderUpdate.vs"))
        return false;

    if (!addShader(GL_GEOMETRY_SHADER, "../shaders/borderUpdate.gs"))
        return false;

	if (!finalize())
		return false;

	const GLchar* varyings[4];
    varyings[0] = "out_type";
    varyings[1] = "out_pos";
    varyings[2] = "out_vel";    
    varyings[3] = "out_age";

    glTransformFeedbackVaryings(program, 4, varyings, GL_INTERLEAVED_ATTRIBS);

    if (!finalize()) {
        return false;
    }

    // get uniform locations
    locDt = getUniformLocation("dt");
    locSpeed = getUniformLocation("speed");
    locGeneratorTime = getUniformLocation("generatorTime");
    locXBorderTime = getUniformLocation("xBorderTime");
    locZBorderTime = getUniformLocation("zBorderTime");

    return true;
}

void BorderUpdateProgram::setDt(float dt){
    
    glUniform1f(locDt, (float)dt);
}

void BorderUpdateProgram::setSpeed(float speed){
    
    glUniform1f(locSpeed, (float)speed);
}

void BorderUpdateProgram::setGeneratorTime(float time){

    glUniform1f(locGeneratorTime, (float)time);
}

void BorderUpdateProgram::setXBorderTime(float time){
    
    glUniform1f(locXBorderTime, (float)time);
}

void BorderUpdateProgram::setZBorderTime(float time){
    
    glUniform1f(locZBorderTime, (float)time);
}