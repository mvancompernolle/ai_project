#include "ExplosionUpdateProgram.hpp"

using namespace Vancom;

ExplosionUpdateProgram::ExplosionUpdateProgram(){

}

bool ExplosionUpdateProgram::init(){

    if (!ShaderProgram::init())
        return false;

    if (!addShader(GL_VERTEX_SHADER, "../shaders/fireworksUpdate.vs"))
        return false;

    if (!addShader(GL_GEOMETRY_SHADER, "../shaders/explosionUpdate.gs"))
        return false;

	if (!finalize())
		return false;

	const GLchar* varyings[4];
    varyings[0] = "type1";
    varyings[1] = "pos1";
    varyings[2] = "vel1";    
    varyings[3] = "age1";

    glTransformFeedbackVaryings(program, 4, varyings, GL_INTERLEAVED_ATTRIBS);

    if (!finalize()) {
        return false;
    }

    locDt = getUniformLocation("dt");
    locCenter = getUniformLocation("center");

    return true;
}

void ExplosionUpdateProgram::setDt(unsigned int dt) const{

    glUniform1f(locDt, (float)dt);
}


void ExplosionUpdateProgram::setCenter(const glm::vec3& center) const{

    glUniform3fv(locCenter, 1, glm::value_ptr(center));
}