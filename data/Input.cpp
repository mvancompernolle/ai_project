#include "Input.hpp"
#include "Engine.hpp"
#include "Graphics.hpp"
#include "EntityManager.hpp"
#include "Camera.hpp"
#include "LightingManager.hpp"

#include <SDL.h>

using namespace Vancom;

Input::Input(Engine *engine) : engine(engine){

    sensitivity = 0.5f;
}

Input::~Input(){

}

void Input::init(){

    SDL_SetRelativeMouseMode(SDL_TRUE);

    resetKeysPressed();
}

void Input::tick(float dt){

    SDL_Event event;
    float x,y;

    while(SDL_PollEvent(&event)) {
        switch(event.type) {

            case SDL_KEYDOWN:

                if(event.key.keysym.mod == KMOD_LSHIFT || event.key.keysym.mod == KMOD_RSHIFT)

                    keysPressed[KMOD_LSHIFT] = true;

                switch(event.key.keysym.sym) {

                    case SDLK_ESCAPE:
                        engine->stop();
                    break;


                    case SDLK_w:
                        keysPressed[SDLK_w] = true;
                    break;

                    case SDLK_s:
                        keysPressed[SDLK_s] = true;
                    break;

                    case SDLK_d:
                        keysPressed[SDLK_d] = true;
                    break;

                    case SDLK_a:
                        keysPressed[SDLK_a] = true;
                    break;

                    case SDLK_r:
                        keysPressed[SDLK_r] = true;
                    break;

                    case SDLK_f:
                        keysPressed[SDLK_f] = true;
                    break;

                    case SDLK_i:
                        keysPressed[SDLK_i] = true;
                    break;

                    case SDLK_u:
                        keysPressed[SDLK_u] = true;
                    break;

                    case SDLK_LEFT:
                        keysPressed[SDLK_LEFT] = true;
                    break;

                    case SDLK_RIGHT:
                        keysPressed[SDLK_RIGHT] = true;
                    break;
                }

            break;

            case SDL_KEYUP:

                if(!(event.key.keysym.mod == KMOD_LSHIFT || event.key.keysym.mod == KMOD_RSHIFT))
                    keysPressed[KMOD_LSHIFT] = false;

                switch(event.key.keysym.sym) {
                    case SDLK_w:
                        keysPressed[SDLK_w] = false;
                    break;

                    case SDLK_s:
                        keysPressed[SDLK_s] = false;
                    break;

                    case SDLK_d:
                        keysPressed[SDLK_d] = false;
                    break;

                    case SDLK_a:
                        keysPressed[SDLK_a] = false;
                    break;

                    case SDLK_r:
                        keysPressed[SDLK_r] = false;
                    break;

                    case SDLK_f:
                        keysPressed[SDLK_f] = false;
                    break;

                    case SDLK_i:
                        keysPressed[SDLK_i] = false;
                    break;

                    case SDLK_u:
                        keysPressed[SDLK_u] = false;
                    break;

                    case SDLK_LEFT:
                        keysPressed[SDLK_LEFT] = false;
                    break;

                    case SDLK_RIGHT:
                        keysPressed[SDLK_RIGHT] = false;
                    break;
                }
            break;

            case SDL_QUIT:
                engine->stop();
            break;

            case SDL_MOUSEMOTION:
                x = (float)event.motion.xrel * sensitivity;
                y = (float)event.motion.yrel * sensitivity;

                engine->graphics->camera->rotate(x,y);
            break;

            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == SDL_BUTTON_LEFT){

                    // create an explosion at each entity
                    for(Entity *entity : engine->input->selected){
                        engine->entityManager->createExplosion(entity->getPos());
                        entity->updating = false;
                    }

                    // clear the selected entities
                    engine->input->selected.clear();
                }
            break;

            case SDL_WINDOWEVENT:
                if(event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    engine->graphics->windowResized();
                }
            break;
        }
    }

    handleMovementKeys();
}

void Input::stop(){

}

void Input::resetKeysPressed(){

    keysPressed[SDLK_w] = false;
    keysPressed[SDLK_a] = false;
    keysPressed[SDLK_s] = false;
    keysPressed[SDLK_d] = false;
    keysPressed[SDLK_r] = false;
    keysPressed[SDLK_f] = false;

    keysPressed[KMOD_LSHIFT] = false;

    keysPressed[SDLK_LEFT] = false;
    keysPressed[SDLK_RIGHT] = false;
}

void Input::handleMovementKeys(){

    if(keysPressed[KMOD_LSHIFT])
        engine->graphics->camera->applyBoost(5.0f);

    else
        engine->graphics->camera->applyBoost(1.0f);

    if(keysPressed[SDLK_w])
        engine->graphics->camera->moveForward();

    if(keysPressed[SDLK_s])
        engine->graphics->camera->moveBackward();

    if(keysPressed[SDLK_d])
        engine->graphics->camera->moveRight();

    if(keysPressed[SDLK_a])
        engine->graphics->camera->moveLeft();

    if(keysPressed[SDLK_r])
        engine->graphics->camera->moveUp();

    if(keysPressed[SDLK_f])
        engine->graphics->camera->moveDown();

    if(keysPressed[SDLK_i])
        engine->graphics->isRaining = true;

    if(keysPressed[SDLK_u])
        engine->graphics->isRaining = false;

    if(keysPressed[SDLK_LEFT]){

        // decrement directional light angle in graphics
        engine->lightingManager->increaseLightAngle();

    }

    if(keysPressed[SDLK_RIGHT]){

        // increment direction light angle in graphics
        engine->lightingManager->decreaseLightAngle();
    }

}