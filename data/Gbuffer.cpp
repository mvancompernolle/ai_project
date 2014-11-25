#include "GBuffer.hpp"

#include <stdio.h>

// SOURCE : http://ogldev.atspace.co.uk/www/tutorial37/tutorial37.html

using namespace Vancom;

GBuffer::GBuffer(){

	fbo = 0;
	depthTexture = 0;
	finalTexture = 0;
}

GBuffer::~GBuffer(){

	if(fbo != 0)
		glDeleteFramebuffers(1, &fbo);
	if(textures[0] != 0)
		glDeleteTextures(GBUFFER_NUM_TEXTURES, textures);
	if(depthTexture != 0)
		glDeleteTextures(1, &depthTexture);
	if(finalTexture != 0)
		glDeleteTextures(1, &finalTexture);
}

bool GBuffer::init(unsigned int windowWidth, unsigned int windowHeight){

	// create the FBO
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	// create the gbuffer textures
	glGenTextures(GBUFFER_NUM_TEXTURES, textures);
	glGenTextures(1, &depthTexture);
	glGenTextures(1, &finalTexture);

    for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
    	glBindTexture(GL_TEXTURE_2D, textures[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textures[i], 0);
    }

    // create smaller size specular texture (vec2)	
	glBindTexture(GL_TEXTURE_2D, textures[GBUFFER_TEXTURE_TYPE_SPECULAR]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, windowWidth, windowHeight, 0, GL_RG, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + GBUFFER_TEXTURE_TYPE_SPECULAR, GL_TEXTURE_2D, 
    	textures[GBUFFER_TEXTURE_TYPE_SPECULAR], 0);

    // depth buffer
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, windowWidth, windowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

    // final texture
    glBindTexture(GL_TEXTURE_2D, finalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, finalTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", status);
        return false;
    }

	// restore default FBO
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return true;
}

void GBuffer::startFrame(){

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glDrawBuffer(GL_COLOR_ATTACHMENT4);
	glClear(GL_COLOR_BUFFER_BIT);
}

void GBuffer::bindForGeometryPass(){

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, 
						     GL_COLOR_ATTACHMENT1,
						     GL_COLOR_ATTACHMENT2,
						     GL_COLOR_ATTACHMENT3 };

    glDrawBuffers(4, drawBuffers);
}

void GBuffer::bindForStencilPass(){

	glDrawBuffer(GL_NONE);
}

void GBuffer::bindForLightPass(){

	glDrawBuffer(GL_COLOR_ATTACHMENT4);

	for (unsigned int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
		glActiveTexture(GL_TEXTURE0 + i);		
		glBindTexture(GL_TEXTURE_2D, textures[GBUFFER_TEXTURE_TYPE_POSITION + i]);
	}
}

void GBuffer::bindForFinalPass(){

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT4);
}