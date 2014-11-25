#include <iostream>
#include "Camera.h"

Camera::Camera(glm::vec3 pos, glm::vec3 orientation, glm::vec3 up, float speed, float sensitivity)
{
  this->pos = pos;
  this->orientation = orientation;
  this->up = up;
  this->speed = speed;
  this->sensitivity = sensitivity;
  rotateX = rotateY = 0.0f;
}

Camera::~Camera(){

}

void Camera::moveForward(){
  pos += (glm::normalize(orientation) * speed);
}

void Camera::moveBackward(){
  pos -= (glm::normalize(orientation) * speed);
}

void Camera::moveRight(){
  pos += glm::cross( (glm::normalize(orientation) * speed), up );
}

void Camera::moveLeft(){
  pos -= glm::cross( (glm::normalize(orientation) * speed), up );
}

void Camera::moveUp(){
  pos += up * speed;
}

void Camera::moveDown(){
  pos -= up * speed;
}

void Camera::rotate(float x, float y){
  rotateX -= x * sensitivity;
  rotateY += y * sensitivity;
}

void Camera::update(){
  if(rotateX != 0.0f){
    orientation = glm::rotate(orientation, rotateX, up);
  }

  if(rotateY != 0.0f){
    orientation = glm::rotate(orientation, rotateY, glm::cross(up, orientation));
  }

  rotateX = rotateY = 0.0f;
}

glm::mat4 Camera::getView() const{
  return glm::lookAt(pos, pos + orientation, up);
}
