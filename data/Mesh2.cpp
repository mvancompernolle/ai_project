#include "Mesh.hpp"

#include <sstream>
#include <string>
#include <iostream>

// REFERENCE: ogldev.atspace.co.uk/www/tutorial22/tutorial22.html

using namespace Vancom;

Mesh::MeshUnit::MeshUnit(){
	numIndices = 0;
	baseVertex = 0;
	baseIndex = 0;
	materialIndex = 0;
}

Mesh::Mesh(){
	withAdjacencies = false;
	vao = 0;
}

bool Mesh::loadMesh(const char* fileName, bool withAdjacencies){

	this->withAdjacencies = withAdjacencies;
	bool success = false;
	Assimp::Importer importer;
	const aiScene* scene;

	// create the vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// create the buffers for the vertices attributes
	glGenBuffers(4, buffers);

	// load the scene
	scene = importer.ReadFile(fileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals /*| aiProcess_JoinIdenticalVertices*/);

	// check to see if scene loaded properly
	if (scene) {
        success = loadMeshesFromScene(scene, fileName);
    }
    else {
        printf("Error parsing '%s': '%s'\n", fileName, importer.GetErrorString());
    }

    // unbind the vao
    glBindVertexArray(0);

    return success;
}

bool Mesh::loadMeshesFromScene(const aiScene* scene, const char* fileName){

	// initialize size of mesh and texture arrays
	meshes.resize(scene->mNumMeshes);
	textures.resize(scene->mNumMaterials);

	// prepare vectors for vertex attributes and indices
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> texCoords;
	std::vector<unsigned int> indices;

	unsigned int numVertices = 0;
	unsigned int numIndices = 0;

	unsigned int verticesPer = withAdjacencies ? 6 : 3;

	// count the number of vertices and indices
	for(unsigned int i=0; i< meshes.size(); i++){
		meshes[i].materialIndex = scene->mMeshes[i]->mMaterialIndex;
		meshes[i].numIndices = scene->mMeshes[i]->mNumFaces * verticesPer;
		meshes[i].baseVertex = numVertices;
		meshes[i].baseIndex = numIndices;

		numVertices += scene->mMeshes[i]->mNumVertices;
		numIndices += meshes[i].baseIndex;
	}

	// reserve the space in the vectors for the vertex attribs and indices
	positions.reserve(numVertices);
	normals.reserve(numVertices);
	texCoords.reserve(numVertices);
	indices.reserve(numIndices);

	// initialize each mesh in the scene
	for(unsigned int i=0; i < meshes.size(); i++){
		const aiMesh* mesh = scene->mMeshes[i];
		initializeMesh(mesh, positions, normals, texCoords, indices);
	}

	// get the textures for the meshes
	if(!initializeMaterials(scene, fileName)){
		return false;
	}

	// generate and populate the buffers with vertex attributes and the indices
  	glBindBuffer(GL_ARRAY_BUFFER, buffers[POS_VB]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions[0]) * positions.size(), &positions[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);    

    glBindBuffer(GL_ARRAY_BUFFER, buffers[TEXCOORD_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords[0]) * texCoords.size(), &texCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

   	glBindBuffer(GL_ARRAY_BUFFER, buffers[NORMAL_VB]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffers[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

	return true;
}

void Mesh::initializeMesh(const aiMesh* mesh, std::vector<glm::vec3>& positions, 
		std::vector<glm::vec3>& normals, std::vector<glm::vec2>& texCoords, std::vector<unsigned int>& indices){

	const aiVector3D zero (0.0f, 0.0f, 0.0f);

	// populate the vertex attrib vectors
	for(unsigned int i=0; i < mesh->mNumVertices; i++){
		const aiVector3D* pos = &(mesh->mVertices[i]);
		const aiVector3D* normal = &(mesh->mNormals[i]);
		const aiVector3D* texCoord = mesh->HasTextureCoords(0) ? &(mesh->mTextureCoords[0][i]) : &zero;

		positions.push_back(glm::vec3(pos->x, pos->y, pos->z));
		normals.push_back(glm::vec3(normal->x, normal->y, normal->z));
		texCoords.push_back(glm::vec2(texCoord->x, texCoord->y));
	}

	if(withAdjacencies){
		findAdjacencies(mesh, indices);
	}
	else{
		// populate the index vector
		for(unsigned int i=0; i<mesh->mNumFaces; i++){
			const aiFace& face = mesh->mFaces[i];
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
	}
}

void Mesh::findAdjacencies(const aiMesh* mesh, std::vector<unsigned int>& indices){

	for(int i=0; i<mesh->mNumFaces; i++){
		const aiFace& face = mesh->mFaces[i];

		Face unique;

		// if a position vector is duplicated in the VB we fetch the index of the first occurrence
		for(int j=0; j<3; j++){
			int index = face.mIndices[j];
			aiVector3D& v = mesh->mVertices[index];

			if(posMap.find(v) == posMap.end()){
				posMap[v] = index;
			}
			else{
				index = posMap[v];
			}

			unique.indices[j] = index;
		}

		uniqueFaces.push_back(unique);

		Edge e1(unique.indices[0], unique.indices[1]);
		Edge e2(unique.indices[1], unique.indices[2]);
		Edge e3(unique.indices[2], unique.indices[0]);	

		indexMap[e1].addNeighbor(i);
		indexMap[e2].addNeighbor(i);
		indexMap[e3].addNeighbor(i);	
	}

	// build the index buffer with the adjacency info
	for(int i=0; i<mesh->mNumFaces; i++){
		const Face& face = uniqueFaces[i];

		for(int j=0; j<3; j++){
			Edge e(face.indices[j], face.indices[(j + 1) % 3]);
			Neighbors n = indexMap[e];
			int otherTri = n.getOther(i);

			const Face& otherFace = uniqueFaces[otherTri];
			int oppositeIndex = otherFace.getOppositeIndex(e);

			indices.push_back(face.indices[j]);
			indices.push_back(oppositeIndex);
		}
	}
}

bool Mesh::initializeMaterials(const aiScene* scene, const char* fileName){
	// convert fileName to string
	std::stringstream ss;
	std::string fileString;
	ss << fileName;
	ss >> fileString;

    // Extract the directory part from the file name
    std::string::size_type SlashIndex = fileString.find_last_of("/");
    std::string Dir;

    if (SlashIndex == std::string::npos) {
        Dir = ".";
    }
    else if (SlashIndex == 0) {
        Dir = "/";
    }
    else {
        Dir = fileString.substr(0, SlashIndex);
    }

    bool success = true;

	// initialize the materials
	for(unsigned int i=0; i < scene->mNumMaterials; i++){
		const aiMaterial* material = scene->mMaterials[i];
		
		textures[i] = NULL;
		if(material->GetTextureCount(aiTextureType_DIFFUSE) > 0){
			aiString filePath;
			
			if(material->GetTexture(aiTextureType_DIFFUSE, 0, &filePath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS){
				std::string fullPath = Dir + "/" + filePath.data;
				textures[i] = new Texture(GL_TEXTURE_2D, fullPath.c_str());
				
				if(!textures[i]->create()){
					printf("Error loading the texture '%s'\n", fullPath.c_str());
					delete textures[i];
					textures[i] = NULL;
					success = false;
				}
	            else {
                    printf("Loaded texture '%s'\n", fullPath.c_str());
                }			
			}
		}
	}

	return success;
}

void Mesh::renderMesh()
{
	glBindVertexArray(vao);

	unsigned int topology = withAdjacencies ? GL_TRIANGLES_ADJACENCY : GL_TRIANGLES;

    for (unsigned int i = 0; i < meshes.size(); i++) {

        const unsigned int materialIndex = meshes[i].materialIndex;

        if (materialIndex < textures.size() && textures[materialIndex]) {
            textures[materialIndex]->bind(GL_TEXTURE0);
        }

        glDrawElementsBaseVertex(topology, 
        	meshes[i].numIndices, 
        	GL_UNSIGNED_INT, 
        	(void*)(sizeof(unsigned int) * meshes[i].baseIndex), 
        	meshes[i].baseVertex);
    }

    glBindVertexArray(0);
}




