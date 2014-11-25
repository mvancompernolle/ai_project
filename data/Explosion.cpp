#include "Explosion.hpp"

#include <iostream>
#include <random>

#include "EntityManager.hpp"

#define MAX_PARTICLES 1000

using namespace Vancom;

Explosion::Explosion(EntityManager *mgr) : mgr(mgr){

	// initialize variables
	firstTime = true;
	VBOIndex = 0;
	TBOIndex = 1;
	timeElapsed = 0;
	texture = NULL;
}

Explosion::~Explosion(){

	delete texture;

	if(transformFeedback[0] != 0)
		glDeleteTransformFeedbacks(2, transformFeedback);

	if(particleBuffer[0] != 0)
		glDeleteBuffers(2, particleBuffer);
}

bool Explosion::init(const glm::vec3& pos){

	Particle particles[MAX_PARTICLES];

	// setup random number generator
    std::random_device rd;
    std::default_random_engine gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

	// loop through and randomly create explosion particle
	for(int i=0; i<MAX_PARTICLES; i++){
		particles[i].pos = pos;
		particles[i].vel = glm::normalize(glm::vec3(dist(gen), dist(gen), dist(gen))) * glm::vec3(50, 50, 50);
		particles[i].lifeTimeMS = 0.0f;
	}

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

	//model = glm::translate(model, pos);

	// initialize update program
	if(!updateProgram.init()){
		return false;
	}

	updateProgram.enable();
	updateProgram.setCenter(pos);

	// initialize render program
	if(!renderProgram.init())
		return false;

	renderProgram.enable();
	renderProgram.setColorTextureUnit(0);
	renderProgram.setBillboardSize(5.0f);

	texture = new Texture(GL_TEXTURE_2D, "../assets/models/fireworks_red.jpg");
	if(!texture->create()){
		std::cout << "Failed to load Fireworks texture" << std::endl;
		return false;
	}

	return true;
}

void Explosion::tick(float dt){

	// save dt
	this->dt = dt * 1000;

	// update time
	timeElapsed += this->dt;

	if(timeElapsed >= 3100){
		mgr->explosions.clear();
		delete this;
	}
}

void Explosion::updateParticles(){

	updateProgram.enable();
	updateProgram.setDt(dt);

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
    	glDrawArrays(GL_POINTS, 0, MAX_PARTICLES);
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

void Explosion::renderParticles(glm::mat4 proj, glm::mat4 view, glm::vec3 camPos){

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

void Explosion::render(glm::mat4 projection, glm::mat4 view, glm::vec3 camPos){

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

void Explosion::render(){

}