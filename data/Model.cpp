#include "Model.hpp"

#include <random>

#include "EntityManager.hpp"

using namespace Vancom;

Model::Model(){

	speed = 5;
	timeElapsed = 10;
	decisionTime = 10;
	specularIntensity = 0;
	specularPower = 0;
}

Model::Model(EntityManager *mgr, glm::vec3 pos, float scale, float power, float intensity): mgr(mgr){
	Model();
	moving = true;
	speed = 5;
	timeElapsed = 3;
	decisionTime = 3;
	specularIntensity = power;
	specularPower = intensity;	
	target = pos;
	model = glm::translate(model, pos);
	model = glm::scale(model, glm::vec3(scale, scale, scale));
}

Model::~Model(){

}

bool Model::init(const char* fileName){

	// load Model model from assimp
	mesh = new Mesh();
	mesh->loadMesh(fileName, true);

	return true;
}

void Model::tick(float dt){

	// get current model position
	glm::vec3 pos = glm::vec3(model[3][0], model[3][1], model[3][2]);

	// update target
	timeElapsed += dt;
	if(timeElapsed >= decisionTime || glm::distance(pos, target) < 5.0f){
		timeElapsed = 0;

		// pick a random location to move to 
	    std::random_device rd;
	    std::default_random_engine gen(rd());
	    std::uniform_real_distribution<float> distX(mgr->minX, mgr->maxX);
	    std::uniform_real_distribution<float> distZ(mgr->minZ, mgr->maxZ);
		std::uniform_real_distribution<float> distSpeed(2.0f, 10.0f);

	    target = glm::vec3(distX(gen), pos.y, distZ(gen));
	    speed = distSpeed(gen);
	    	//std::cout << target.x << " " << target.y << " " << target.z << std::endl;
	}

	if(updating)
		model = glm::translate(model, glm::normalize(target - pos) * dt * speed);

}

void Model::render(){

	// draw
	mesh->renderMesh();
}