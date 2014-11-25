#include "ShaderProgram.hpp"

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <streambuf>

using namespace Vancom;

ShaderProgram::ShaderProgram(){

	program = 0;
}

ShaderProgram::~ShaderProgram(){

	for(std::list<GLuint>::iterator it = shaderObjList.begin() ; it != shaderObjList.end(); it++){
		glDeleteShader(*it);
	}

	if(program != 0){
		glDeleteProgram(program);
		program = 0;
	}
}

bool ShaderProgram::init(){

	program = glCreateProgram();

	if(program == 0){
        fprintf(stderr, "Error creating shader program\n");
		return false;
	}

	return true;
}

void ShaderProgram::enable(){

	glUseProgram(program);
}

bool ShaderProgram::addShader(GLenum shaderType, const char* fileName){

    std::ifstream fin(fileName);
	GLint shader_status;
	std::string buf;
	const char * shaderString;

	// read shader from file
	if(fin){

		// seek to the end of the file to get its size
		fin.seekg(0, std::ios::end);
		long size = fin.tellg();
		fin.seekg(0, std::ios::beg);

		// dynamically allocate memory space for the shader string based on file size
		buf.reserve(size);

		// fill string with contents of file
		std::istreambuf_iterator<char> iterator = std::istreambuf_iterator<char>(fin);
		std::istreambuf_iterator<char> iteratorEnd = std::istreambuf_iterator<char>();
		buf.assign(iterator, iteratorEnd);
  		shaderString = buf.c_str();
		fin.close();

		// create and compile the shader
		GLuint shader = glCreateShader(shaderType);

		glShaderSource(shader, 1, &shaderString, NULL);
		glCompileShader(shader);

		//check the compile status
		glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_status);

		if(!shader_status){

			GLint logSize = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

			char msg[logSize];
			glGetShaderInfoLog(shader, logSize, &logSize, msg);
			std::cout << msg << std::endl;
		    
		    return false;
		}
		else{

			// add shader to the shader list and attach to program
			shaderObjList.push_back(shader);
			glAttachShader(program, shader);
			return true;
		}
	}
	else{
        std::cerr << "[F] FAILED TO OPEN SHADER FILE: " << fileName << std::endl;
        return false;
	}


}

bool ShaderProgram::finalize(){

	GLint success = 0;
	GLchar errorLog[1024] = { 0 };

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);

	if(success == 0){
		glGetProgramInfoLog(program, sizeof(errorLog), NULL, errorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", errorLog);
	}

	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);

	if(success == 0){
		glGetProgramInfoLog(program, sizeof(errorLog), NULL, errorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", errorLog);
	}

    // Delete the intermediate shader objects that have been added to the program
    for (std::list<GLuint>::iterator it = shaderObjList.begin() ; it != shaderObjList.end() ; it++) {
        glDeleteShader(*it);
    }

    shaderObjList.clear();

    return true;
}

GLint ShaderProgram::getUniformLocation(const char* uniformName){

    GLuint location = glGetUniformLocation(program, uniformName);

    if (location == -1) {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", uniformName);
    }

    return location;
}

GLint ShaderProgram::getProgramParam(GLint param){

    GLint ret;
    glGetProgramiv(program, param, &ret);
    return ret;
}
