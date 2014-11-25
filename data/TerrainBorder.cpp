#include "TerrainBorder.hpp"

#include <iostream>

#include "Engine.hpp"
#include "Graphics.hpp"
#include "Camera.hpp"

#define MAX_PARTICLES 5000

#define TOP_GENERATOR 0.0f
#define TOP_BORDER 1.0f
#define BOTTOM_GENERATOR 2.0f
#define BOTTOM_BORDER 3.0f
#define RIGHT_GENERATOR 4.0f                                                                                       
#define RIGHT_BORDER 5.0f
#define LEFT_GENERATOR 6.0f                                                                                       
#define LEFT_BORDER 7.0f 

using namespace Vancom;

TerrainBorder::TerrainBorder(Engine *engine, float speed) : engine(engine), speed(speed){

	distToCamera = 0;
	renderDistance = 0;
	firstTime = true;
	isRendering = false;
	VBOIndex = 0;
	TBOIndex = 1;
	dt = 0;
}

TerrainBorder::~TerrainBorder(){

	// delete buffers
	if(transformFeedback[0] != 0)
		glDeleteTransformFeedbacks(2, transformFeedback);

	if(particleBuffer[0] != 0)
		glDeleteBuffers(2, particleBuffer);
}

bool TerrainBorder::init(float renderDistance, glm::vec2 topLeft, glm::vec2 topRight, glm::vec2 bottomLeft, glm::vec2 bottomRight){

	// create initial particles
	Particle particles[MAX_PARTICLES];

	// save topleft and bottom right position
	this->topLeft = topLeft;
	this->bottomRight = bottomRight;

	// save render distance
	this->renderDistance = renderDistance;

	// init particles so that they fill up the screen
	initParticles(particles);

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

	// initialize update program
	if(!updateProgram.init())
		return false;

	updateProgram.enable();

	// set uniforms
	updateProgram.setSpeed(speed);
	updateProgram.setGeneratorTime(200.0f);
	updateProgram.setXBorderTime((topRight.x - topLeft.x)/speed);
	updateProgram.setZBorderTime((topRight.y - bottomRight.y)/speed);

	// initialize render program
	if(!renderProgram.init())
		return false;

	renderProgram.enable();
	renderProgram.setParticleSize(12, 2);

	return true;
}

void TerrainBorder::initParticles(Particle* particles){
	// initialize a particle in the top left
	particles[0].type = TOP_GENERATOR;
	particles[0].pos = glm::vec3(topLeft.x, 0.0, topLeft.y);
	particles[0].vel = glm::vec3(0.0f, 0.0f, 0.0f);
	particles[0].lifeTimeMS = 0;

	particles[1].type = BOTTOM_GENERATOR;
	particles[1].pos = glm::vec3(bottomRight.x, 0.0, bottomRight.y);
	particles[1].vel = glm::vec3(0.0f, 0.0f, 0.0f);
	particles[1].lifeTimeMS = 0;

	particles[2].type = RIGHT_GENERATOR;
	particles[2].pos = glm::vec3(bottomRight.x, 0.0, topLeft.y);
	particles[2].vel = glm::vec3(0.0f, 0.0f, 0.0f);
	particles[2].lifeTimeMS = 0;

	particles[3].type = LEFT_GENERATOR;
	particles[3].pos = glm::vec3(topLeft.x, 0.0, bottomRight.y);
	particles[3].vel = glm::vec3(0.0f, 0.0f, 0.0f);
	particles[3].lifeTimeMS = 0;

	particleCount = 4;
	float pos = 0;
	float dist = speed * 200;
	int count = 0;

	// initialize all top border particles
	while(pos <= (bottomRight.x - topLeft.x)/speed){
		particles[count+particleCount].type = TOP_BORDER;
		particles[count+particleCount].pos = glm::vec3(topLeft.x + pos, 0.0, topLeft.y);
		particles[count+particleCount].vel = glm::vec3(speed, 0.0f, 0.0f);
		particles[count+particleCount].lifeTimeMS = count * 200;
		pos += dist;
		count++;
	}

	particleCount += count;
	count = 0;
	pos = 0;
	// initialize all bottom border particles
	while(pos <= (bottomRight.x - topLeft.x)/speed){
		particles[count+particleCount].type = BOTTOM_BORDER;
		particles[count+particleCount].pos = glm::vec3(bottomRight.x - pos, 0.0, bottomRight.y);
		particles[count+particleCount].vel = glm::vec3(-speed, 0.0f, 0.0f);
		particles[count+particleCount].lifeTimeMS = count * 200;
		pos += dist;
		count++;
	}

	particleCount += count;
	count = 0;
	pos = 0;
	// initialize all right border particles
	while(pos <= (topLeft.y - bottomRight.y)/speed){
		particles[count+particleCount].type = RIGHT_BORDER;
		particles[count+particleCount].pos = glm::vec3(bottomRight.x, 0.0, topLeft.y - pos);
		particles[count+particleCount].vel = glm::vec3(0.0f, 0.0f, -speed);
		particles[count+particleCount].lifeTimeMS = count * 200;
		pos += dist;
		count++;
	}

	particleCount += count;
	count = 0;
	pos = 0;
	// initialize all right border particles
	while(pos <= (topLeft.y - bottomRight.y)/speed){
		particles[count+particleCount].type = LEFT_BORDER;
		particles[count+particleCount].pos = glm::vec3(topLeft.x, 0.0, bottomRight.y + pos);
		particles[count+particleCount].vel = glm::vec3(0.0f, 0.0f, +speed);
		particles[count+particleCount].lifeTimeMS = count * 200;
		pos += dist;
		count++;
	}

	particleCount += count;
}

void TerrainBorder::tick(float dt){

	// save dt
	this->dt = dt * 1000;

	// get camera postion
	glm::vec3 camPos = engine->graphics->camera->getPos();

	// make sure that the camera is in bounds of the border
	// max z
	if(camPos.z > topLeft.y)
		engine->graphics->camera->setPos(glm::vec3(camPos.x, camPos.y, topLeft.y));
	// min z
	else if(camPos.z < bottomRight.y)
		engine->graphics->camera->setPos(glm::vec3(camPos.x, camPos.y, bottomRight.y));
	// max x
	if(camPos.x > bottomRight.x)
		engine->graphics->camera->setPos(glm::vec3(bottomRight.x, camPos.y, camPos.z));
	// max y
	else if(camPos.x < topLeft.x)
		engine->graphics->camera->setPos(glm::vec3(topLeft.x, camPos.y, camPos.z));

	// get the cameras distance from the borders and only render if close
	// distance max z
	if((distToCamera = (topLeft.y - camPos.z)) < renderDistance){
		isRendering = true;
	}
	// distance min z
	else if((distToCamera = (camPos.z - bottomRight.y)) < renderDistance){
		isRendering = true;
	}
	// distance max x
	else if((distToCamera = (bottomRight.x - camPos.x)) < renderDistance){
		isRendering = true;
	}
	// distance min x
	else if((distToCamera = (camPos.x - topLeft.x)) < renderDistance){
		isRendering = true;
	}
	else{
		isRendering = false;
	}
	//std::cout << dist << std::endl;
}

void TerrainBorder::updateParticles(){
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
    glVertexAttribPointer(3,1,GL_FLOAT,GL_FALSE,sizeof(Particle),(const GLvoid*)28); // age

    glBeginTransformFeedback(GL_POINTS);

    if(firstTime){
    	glDrawArrays(GL_POINTS, 0, particleCount);
    	firstTime = false;
    }
    else{
    	glDrawTransformFeedback(GL_POINTS, transformFeedback[VBOIndex]);
    }

    glEndTransformFeedback();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

void TerrainBorder::renderParticles(glm::mat4 proj, glm::mat4 view){
	renderProgram.enable();

	// set uniforms
	renderProgram.setMVP(proj * view);
	renderProgram.setCameraPos(engine->graphics->camera->getPos());
	renderProgram.setMixVal((renderDistance - distToCamera)/renderDistance);

	// enable resterizer
	glDisable(GL_RASTERIZER_DISCARD);

	glBindBuffer(GL_ARRAY_BUFFER, particleBuffer[TBOIndex]);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)0);  // type
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Particle), (const GLvoid*)4);  // position

    glDrawTransformFeedback(GL_POINTS, transformFeedback[TBOIndex]);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
}

void TerrainBorder::render(glm::mat4 projection, glm::mat4 view){

	// only render if camera close to edges (determined in tick)
	if(isRendering){
		// might need vao to work
		glBindVertexArray(vao);

		// use time to update particles
		updateParticles();

		// render the particles
		renderParticles(projection, view);

		// swap the buffer indices for the next draw
		VBOIndex = TBOIndex;
		TBOIndex = (TBOIndex + 1) & 0x1;
	}
}

void TerrainBorder::render(){

}