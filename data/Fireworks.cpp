#include "Fireworks.hpp"
#include <iostream>

#define MAX_PARTICLES 20000

#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f

using namespace Vancom;

Fireworks::Fireworks(){
	// initialize variables
	firstTime = true;
	VBOIndex = 0;
	TBOIndex = 1;
	timeElapsed = 0;
	texture = NULL;
}

Fireworks::~Fireworks(){
	delete texture;

	if(transformFeedback[0] != 0)
		glDeleteTransformFeedbacks(2, transformFeedback);

	if(particleBuffer[0] != 0)
		glDeleteBuffers(2, particleBuffer);
}

bool Fireworks::init(const glm::vec3& pos){
	Particle particles[MAX_PARTICLES];

	// create the particle launcher
	particles[0].type = PARTICLE_TYPE_LAUNCHER;
	particles[0].pos = pos;
	particles[0].vel = glm::vec3(0.0f, 0.0001f, 0.0f);
	particles[0].lifeTimeMS = 0.0f;

	// create vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// create buffers that will be swapped back and forth (input/output)
	glGenTransformFeedbacks(2, transformFeedback);
	glGenBuffers(2, particleBuffer);
	for(int i=0; i < 2; i++){
		glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[i]);
		glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(particles), particles, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, particleBuffer[i]);
	}

	model = glm::translate(model, pos);

	// initialize update program
	if(!updateProgram.init()){
		return false;
	}

	updateProgram.enable();
	updateProgram.setRandomTextureUnit(3);
	updateProgram.setLauncherLifetime(250.0f);
	updateProgram.setShellLifetime(1500.0f);
	updateProgram.setSecondaryShellLifetime(2000.0f);
	randomTexture.initRandomTexture(1000);
	randomTexture.bind(GL_TEXTURE3);

	// initialize render program
	if(!renderProgram.init())
		return false;

	renderProgram.enable();
	renderProgram.setColorTextureUnit(0);
	renderProgram.setBillboardSize(2.0f);

	texture = new Texture(GL_TEXTURE_2D, "../assets/models/fireworks_red.jpg");
	if(!texture->create()){
		std::cout << "Failed to load Fireworks texture" << std::endl;
		return false;
	}

	return true;
}

void Fireworks::tick(float dt){
	// save dt
	this->dt = dt * 1000;

	// update time
	timeElapsed += this->dt;
}

void Fireworks::updateParticles(){
	updateProgram.enable();
	updateProgram.setTime(timeElapsed);
	updateProgram.setDt(dt);

	randomTexture.bind(GL_TEXTURE3);

	// disable rendering on update draw
	glEnable(GL_RASTERIZER_DISCARD);

	glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[VBOIndex]);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, transformFeedback[TBOIndex]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(0,1,GL_FLOAT,GL_FALSE,sizeof(Particle),0); // type
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Particle),(const GLvoid*)4); // position
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,sizeof(Particle),(const GLvoid*)16); // velocity
    glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(Particle),(const GLvoid*)28); // lifetime

    glBeginTransformFeedback(GL_POINTS);

    if(firstTime){
    	glDrawArrays(GL_POINTS, 0, 1);
    	firstTime = false;
    }
    else
    	glDrawTransformFeedback(GL_POINTS, transformFeedback[VBOIndex]);

    glEndTransformFeedback();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

void Fireworks::renderParticles(glm::mat4 proj, glm::mat4 view, glm::vec3 camPos){
	renderProgram.enable();
	renderProgram.setMVP(proj * view * model);
	renderProgram.setCameraPos(camPos);
	texture->bind(GL_TEXTURE0);

	// enable resterizer
	glDisable(GL_RASTERIZER_DISCARD);

	glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[TBOIndex]);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);  // position

    glDrawTransformFeedback(GL_POINTS, transformFeedback[TBOIndex]);

    glDisableVertexAttribArray(0);
}

void Fireworks::render(glm::mat4 projection, glm::mat4 view, glm::vec3 camPos){

	// bind vao
	glBindVertexArray(vao);

	// use time to update particles
	updateParticles();

	// render the particles
	renderParticles(projection, view, camPos);

	// swap the buffer indices for the next draw
	VBOIndex = TBOIndex;
	TBOIndex = (TBOIndex + 1) & 0x1;
}

void Fireworks::render(){

}