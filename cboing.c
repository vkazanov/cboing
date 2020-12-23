#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>

#define HEIGHT 800
#define WIDTH 480
#define TITLE "CBoing!"


int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        goto err_init;
    }

    SDL_Window *window = SDL_CreateWindow(
        TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WIDTH, HEIGHT, SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        goto err_window;
    }

    SDL_Surface *screen_surface = SDL_GetWindowSurface(window);
    SDL_FillRect(screen_surface, NULL, SDL_MapRGB(screen_surface->format, 0xFF, 0xFF, 0xFF));
    SDL_UpdateWindowSurface(window);
    SDL_Delay(2000);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;

err_window:
    SDL_DestroyWindow(window);
err_init:
    SDL_Quit();

    return EXIT_FAILURE;
}
