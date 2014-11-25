
#include "ShaderLoader.h"
#include <string>
#include <fstream>
#include <streambuf>

using namespace std;

ShaderLoader:: ShaderLoader(GLenum shdrType){
	shader = 0;
	shaderType = shdrType;
}

bool ShaderLoader:: loadShader(const char *fname){
	

	std::ifstream fin(fname);
	std::string shaderStr;
	GLint shader_status;	
	



	// check file
	if(!fin){
		std::cout << "ERROR: Shader file: "<< fname <<" cannot be opened" << endl;
		return false;
	}
	// grab content from beginning to end of file
	fin.seekg(0, std::ios::end);
	// set size of string
	shaderStr.reserve(fin.tellg());

	//reset file cursor to beginning of file
	fin.seekg(0, std::ios::beg);
	
	//store the content into shaderStr
	shaderStr.assign(std::istreambuf_iterator<char>(fin),
						std::istreambuf_iterator<char>());



	fin.close();

	shader = glCreateShader(shaderType);

	const char *shaderInfo = shaderStr.c_str();

    //compile the shader
    glShaderSource(shader, 1, &shaderInfo, NULL);
    glCompileShader(shader);
    //check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shader_status);



    if(!shader_status)
    {
		if(shaderType == GL_VERTEX_SHADER)
		{
        std::cerr << "[F] FAILED TO COMPILE VERTEX SHADER!" << std::endl;
        return false;
    	}
		else if (shaderType == GL_FRAGMENT_SHADER)
		{
		  std::cerr << "[F] FAILED TO COMPILE FRAGMENT SHADER!" << std::endl;
        return false;
		}

	 }



	return true;

}
		
GLuint ShaderLoader:: getShader() const{
	
	return shader;
}


