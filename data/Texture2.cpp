#include <iostream>
#include "Texture.hpp"

// REFERENCE: ogldev.atspace.co.uk/www/tutorial16/tutorial16.html

using namespace Vancom;

Texture::Texture(GLenum TextureTarget, const char* fileName) : textureTarget(TextureTarget), fileName(fileName){
    
    image = NULL;
}

fipImage Texture::getImage(){

    return image;
}

bool Texture::load(){

    // if texture found, load it
    if(image.load(fileName)) {

        // if unknown image type, return
        if(image.getImageType() == FIT_UNKNOWN) {
            std::cerr << "Unkown image type!" << std::endl;
            return false;
        }

        // convert image to 32 bit pixels
        image.convertTo32Bits();
        image.flipHorizontal();
    }
    else{
        return false;
    }
	
    return true;
}

bool Texture::create(){

    // if texture found, load it
    if(load()) {

        // generate OpenGL texture
        glGenTextures(1, &textureObj);

        // bind texture to textureID
        glBindTexture(textureTarget, textureObj);

        // set texture parameters
        glTexParameterf(textureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(textureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // load OpenGL texture
        glTexImage2D(textureTarget, 0, GL_RGBA, image.getWidth(), image.getHeight(),
            0, GL_BGRA, GL_UNSIGNED_BYTE, (void*) image.accessPixels());
    }
    else{
        return false;
    }
    
    return true;
}

void Texture::bind(GLenum TextureUnit){
	
    glActiveTexture(TextureUnit);
    glBindTexture(textureTarget, textureObj);
	
}
