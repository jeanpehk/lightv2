#include <stdio.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_video.h>

const int WINDOW_WIDTH  = 1024;
const int WINDOW_HEIGHT = 720;

int main(int argc, char *argv[]) {
    bool success = SDL_Init(SDL_INIT_VIDEO);
    if (!success) {
        printf("SDL_Init failed: %s\n", SDL_GetError());

        return 1;
    }

    printf("SDL Initialized\n");

    SDL_Window *window = SDL_CreateWindow("Window",
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_RESIZABLE
    );
    if (!window) {
        printf("Window creation failed: %s\n", SDL_GetError());

        return 1;
    }

    bool done = false;

    while (!done) {
        // Handle events.

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                done = true;
            }
        }

        const bool *kb_state = SDL_GetKeyboardState(NULL);

        if (kb_state[SDL_SCANCODE_ESCAPE]) {
            done = true;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}