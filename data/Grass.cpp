#include <iostream>
#include "Grass.h"

// REFERENCE: ogldev.atspace.co.uk/www/tutorial16/tutorial16.html

Grass::Grass(GLuint prog, float percent, float heightScalar, float widthScale, glm::vec3 color)
{
    this->program = prog;
    this->heightScalar = heightScalar;
    this->widthScalar = widthScale;
    this->percent = percent * 2/3;
    this->color = color;
    loc_position = glGetAttribLocation(program, "v_position");
    loc_gcolor = glGetAttribLocation(program, "v_color");
    loc_offset = glGetAttribLocation(program, "v_offset");
    loc_color = glGetUniformLocation(program, "color");
    loc_hscalar = glGetUniformLocation(program, "verticalScalar");
    loc_wscalar = glGetUniformLocation(program, "terrainScalar");
    loc_mvp = glGetUniformLocation(program, "mvpMatrix");

    // create vbo for grass
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

}

Grass::~Grass(){

}

void Grass::sampleVertices(const std::vector<VertexTexture>& verts){
    vertices.clear();
    VertexGrass vert;
    int perc = percent * 100;
    int index = 0;
    int minGreen = 100, maxGreen = 160, minRed = 28, maxRed = 140, minBlue = 0, maxBlue = 80; 
    std::random_device rd;
    std::default_random_engine rand(rd());
    std::uniform_int_distribution<int> distr(0, 100);
    for(const VertexTexture& v: verts){
        if(distr(rand) < (perc - perc * v.position[1]* 1.5) && index % 3 != 0 && index %4 != 0){
            for(int i=0; i<3; i++)
                vert.position[i] = v.position[i];
            vert.color[0] = ((float)minRed + (maxRed - minRed)*(distr(rand)/(float)100))/255;
            vert.color[1] = ((float)minGreen +(maxGreen - minGreen)*(distr(rand)/(float)100))/255;
            vert.color[2] = ((float)minBlue +(maxBlue - minBlue)*(distr(rand)/(float)100))/255;
            vert.offset = (float) distr(rand) / 300;
            //std::cout << vert.offset << std::endl;
            vertices.push_back(vert);
        }
        index++;
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexGrass) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
}

void Grass::render(glm::mat4 pvMat){
    glm::mat4 mvp = pvMat * model;

    // enable the shader program
    glUseProgram(program);

    glEnableVertexAttribArray(loc_position);
    glEnableVertexAttribArray(loc_gcolor);
    glEnableVertexAttribArray(loc_offset);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    //upload the matrix to the shader
    glUniformMatrix4fv(loc_mvp, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform3fv(loc_color, 1, glm::value_ptr(color));
    glUniform1f(loc_hscalar, heightScalar);
    glUniform1f(loc_wscalar, widthScalar);

    glVertexAttribPointer( loc_position,//  location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(VertexGrass),//stride
                           0);//offset

    glVertexAttribPointer( loc_gcolor,//location of attribute
                           3,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(VertexGrass),//stride
                           (void*) (3*sizeof(float)));//offset

    glVertexAttribPointer( loc_offset,//location of attribute
                           1,//number of elements
                           GL_FLOAT,//type
                           GL_FALSE,//normalized?
                           sizeof(VertexGrass),//stride
                           (void*) (6*sizeof(float)));//offset

    glDrawArrays(GL_POINTS, 0, vertices.size());
    
    glDisableVertexAttribArray(loc_position);
    glDisableVertexAttribArray(loc_gcolor);
    glDisableVertexAttribArray(loc_offset);
    glUseProgram(0);
}