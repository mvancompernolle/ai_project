#include "Camera.hpp"
#include "Engine.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

using namespace Vancom;

Camera::Camera(Engine *engine)
    : engine(engine),
      pos(-200.0f, 150.0f, 50.0f),
      orientation(200.0f, -50.0f, -50.0f),
      up(0.0f,1.0f,0.0f){

    rotateX = rotateY = 0.0f;
    boost = 1.0f;
    speed = 2.0f;
    sensitivity = 0.1f;
}

void Camera::moveForward(){

    pos += (glm::normalize(orientation) * speed * boost);
}

void Camera::moveBackward(){

    pos -= (glm::normalize(orientation) * speed * boost);
}

void Camera::moveLeft(){
    glm::vec3 temp = glm::cross((glm::normalize(orientation) * speed * boost), up);
    pos -= temp * speed/glm::length(temp);
}

void Camera::moveRight(){
    glm::vec3 temp = glm::cross((glm::normalize(orientation) * speed * boost), up);
    pos += temp * speed/glm::length(temp);
}

void Camera::moveUp(){

    pos += (up * speed * boost);
}

void Camera::moveDown(){

    pos -= (up * speed * boost);
}

void Camera::rotate(float x, float y){

    rotateX -= x * sensitivity;
    rotateY += y * sensitivity;
}

void Camera::update(){

    if(rotateX != 0.0f)
        orientation = glm::rotate(orientation, rotateX, up);

    if(rotateY != 0.0f)
        orientation = glm::rotate(orientation, rotateY, glm::cross(up, orientation));

    rotateX = rotateY = 0.0f;
}

glm::mat4 Camera::getView() const{

    return glm::lookAt(pos,
                       pos + orientation,
                       up);
}

void Camera::applyBoost(float newBoost){

  boost = newBoost;
}

void Camera::setPos(glm::vec3 position){

    pos = position;
}

glm::vec3 Camera::getPos() const{

	return pos;
}

glm::vec3 Camera::getCameraDirection() const{

  return glm::normalize(orientation);
}