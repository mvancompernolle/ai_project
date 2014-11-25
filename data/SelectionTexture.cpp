#include "SelectionTexture.hpp"

#include <iostream>
#include <stdio.h>

// Source : http://ogldev.atspace.co.uk/www/tutorial29/tutorial29.html

using namespace Vancom;

SelectionTexture::SelectionTexture(){

	fbo = 0;
	selectionTexture = 0;
	depthTexture = 0;
}

SelectionTexture::~SelectionTexture(){

	// delete members if they exist
    if (fbo != 0) {
        glDeleteFramebuffers(1, &fbo);
    }

    if (selectionTexture != 0) {
        glDeleteTextures(1, &selectionTexture);
    }
    
    if (depthTexture != 0) {
        glDeleteTextures(1, &depthTexture);
    }
}

bool SelectionTexture::init(unsigned int width, unsigned int height){

    // Create the FBO
    glGenFramebuffers(1, &fbo);    
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create the texture object for the primitive information buffer
    glGenTextures(1, &selectionTexture);
    glBindTexture(GL_TEXTURE_2D, selectionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, selectionTexture, 0);    

    // Create the texture object for the depth buffer
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);    

	glReadBuffer(GL_NONE);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

    // Verify that the FBO is correct
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }
    
    // Restore the default framebuffer
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}

void SelectionTexture::enableWriting(){

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
}

void SelectionTexture::disableWriting(){

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

SelectionTexture::PixelInfo SelectionTexture::readPixel(unsigned int x, unsigned int y)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    PixelInfo pixel;

    glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, &pixel);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    return pixel;
}