#include "ModelLoader.h"
#include <cstdlib>
#include <sstream>

ModelLoader:: ModelLoader()
{

}

std::vector<Vertex> ModelLoader:: loadObj(const char * obj, int &size)
{

    Assimp::Importer importer;
    std::string error;
    Vertex vert;
    std::vector<Vertex> geometry;
    aiMaterial *material;
    float defaultColor = .5f;
    aiColor3D color(0.0f, 0.0f, 0.0f);  

    const aiScene* scene = importer.ReadFile(obj, aiProcess_Triangulate);

    // if model loaded correctly
    if (scene) {
        
        for (unsigned int i = 0; i < scene->mNumMeshes; i++){
            // get meshes
            aiMesh* mesh = scene->mMeshes[i];

            if(scene->mNumMaterials > 1){
                material = scene->mMaterials[mesh->mMaterialIndex];
                material->Get(AI_MATKEY_COLOR_DIFFUSE, color);


                // assign x, y, and z colors
                vert.color[0] = color.r;
                vert.color[1] = color.g;
                vert.color[2] = color.b;
            }
            else{

                // default color if no material file
                for( int i = 0; i < 3; i++)
                    vert.color[i] = defaultColor;
                    defaultColor+= .02;
                    if(defaultColor>1)
                        defaultColor=0;


            }

            size += mesh->mNumFaces;


            for(unsigned int j = 0; j < mesh->mNumFaces; j++){
                // get a face from the mesh
                const aiFace face = mesh->mFaces[j];

                for(unsigned int k = 0; k < face.mNumIndices; k++){
                    // get one vertex in the face
                    const aiVector3D vertex = mesh->mVertices[face.mIndices[k]];

                    // get x, y, and z coordinates
                    for(unsigned int l = 0; l < face.mNumIndices; l++)
                        vert.position[l] = vertex[l];

                    // add vertex to geometry
                    geometry.push_back(vert);
                }
            }



        }
    }
    else {
        // error
        error = importer.GetErrorString();
        std::cout<< "Error parsing the model: " << obj << " " << error <<"\n" << std::endl;
        exit(0);
    }

    return geometry;
}

void loadPGMTerrain(const char * obj){
    FREE_IMAGE_FORMAT formato = FreeImage_GetFileType(obj,0);//Automatocally detects the format(from over 20 formats!)
    FIBITMAP* imagen = FreeImage_Load(formato, obj);

    FIBITMAP* temp = imagen;
    imagen = FreeImage_ConvertTo32Bits(imagen);
    FreeImage_Unload(temp);

    int w = FreeImage_GetWidth(imagen);
    int h = FreeImage_GetHeight(imagen);
    //cout<<"The size of the image is: "<<textureFile<<" es "<<w<<"*"<<h<<endl; //Some debugging code
 
    GLubyte* textura = new GLubyte[4*w*h];
    char* pixeles = (char*)FreeImage_GetBits(imagen);
    //FreeImage loads in BGR format, so you need to swap some bytes(Or use GL_BGR).

    for(int j= 0; j<w*h; j++){
        textura[j*4+0]= pixeles[j*4+2];
        textura[j*4+1]= pixeles[j*4+1];
        textura[j*4+2]= pixeles[j*4+0];
        textura[j*4+3]= pixeles[j*4+3];
        //cout<<j<<": "<<textura[j*4+0]<<"**"<<textura[j*4+1]<<"**"<<textura[j*4+2]<<"**"<<textura[j*4+3]<<endl;
    }

    //Now generate the OpenGL texture object 
    GLuint texturaID;
    glGenTextures(1, &texturaID);
    glBindTexture(GL_TEXTURE_2D, texturaID);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, w, h, 0, GL_RGBA,GL_UNSIGNED_BYTE,(GLvoid*)textura );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}
