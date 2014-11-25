#include "CrossHair.hpp"
#include "Texture.hpp"

#include <iostream>

using namespace Vancom;

CrossHair::CrossHair(glm::vec3 pos){
	model = glm::translate(model, pos);
}

CrossHair::~CrossHair(){

}

bool CrossHair::init(const char* fileName, float width, float height){

	// set texture
	texture = new Texture(GL_TEXTURE_2D, fileName);

	if(!texture->create()){
		std::cout << "There was an error creating the crosshair texture" << std::endl;
		return false;
	}

	// create the geometry
	VertexT vert;

	vert.pos = glm::vec3(-width, height, 0);
	vert.tex = glm::vec2(0, 1);
	box.push_back(vert);

	vert.pos = glm::vec3(-width, -height, 0);
	vert.tex = glm::vec2(0, 0);
	box.push_back(vert);

	vert.pos = glm::vec3(width, height, 0);
	vert.tex = glm::vec2(1, 1);
	box.push_back(vert);

	vert.pos = glm::vec3(width, -height, 0);
	vert.tex = glm::vec2(1, 0);
	box.push_back(vert);

	// create vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// create vbo
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VertexT) * box.size(), box.data(), GL_STATIC_DRAW);

	// setup attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexT), 0); // pos
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VertexT), (const GLvoid*) 12); // tex

	return true;
}

void CrossHair::tick(float dt){

}

void CrossHair::render(){

	// bind vao
	glBindVertexArray(vao);

	// enable attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	// bind ground texture
	texture->bind(GL_TEXTURE0);

	// draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, box.size());

	// disable attributes
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}