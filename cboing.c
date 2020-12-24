#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>
#include <SDL_image.h>

#define HEIGHT 480
#define WIDTH 800
#define TITLE "CBoing!"

#define HALF_WIDTH (WIDTH / 2)
#define HALF_HEIGHT (HEIGHT / 2)

#define PLAYER_SPEED 6
#define MAX_AI_SPEED 6

enum {
    MEDIA_MENU0,
    MEDIA_MENU1,
    MEDIA_OVER,
    MEDIA_TABLE,
    MEDIA_NUM,
};

char *media_to_path[MEDIA_NUM] = {
    [MEDIA_MENU0] = "images/menu0.png",
    [MEDIA_MENU1] = "images/menu1.png",
    [MEDIA_OVER] = "images/over.png",
    [MEDIA_TABLE] = "images/table.png",
};

SDL_Surface *game_media[MEDIA_NUM];

enum {
    STATE_MENU = 1,
    STATE_PLAY = 2,
    STATE_GAME_OVER = 3,
} state = STATE_MENU;

typedef struct bat_t {
} bat_t;

typedef struct ball_t {
} ball_t;

typedef struct impact_t {
} impact_t;

typedef struct game_t {
    bat_t bats[2];
    ball_t ball;
    impact_t *impact_list;
    int ai_offset;
} game_t;


void game_reset(void)
{
}

void game_update(void)
{
}

void game_draw(SDL_Surface *target_surface)
{
    /* background */
    SDL_Surface *table_surface = game_media[MEDIA_TABLE];
    SDL_BlitSurface(table_surface, NULL, target_surface, NULL);

    /* 'just scored' */
    /* TODO: */

    /* bats, ball, impacts */
    /* TODO: */

    /* scores */
    /* TODO: */

}

game_t game;
int num_players = 1;
bool space_down = false;

SDL_Surface *load_surface(const char *path, SDL_Surface *screen_surface)
{
    SDL_Surface* loaded_surface = IMG_Load(path);
    if(!loaded_surface) {
        fprintf(stderr, "Unable to load image %s! SDL2_image Error: %s\n", path, IMG_GetError());
        goto err_load;
    }

    SDL_Surface* optimized_surface = SDL_ConvertSurface(loaded_surface, screen_surface->format, 0);
    if (!optimized_surface){
        printf( "Unable to optimize image %s! SDL Error: %s\n", path, SDL_GetError() );
        goto err_optimize;
    }
    return loaded_surface;

err_optimize:
    SDL_FreeSurface( loaded_surface );
err_load:
    return NULL;
}

bool load_media(SDL_Surface *screen_surface)
{
    bool success = true;
    for (int i = 0; i < MEDIA_NUM; ++i) {
        SDL_Surface *surface = load_surface(media_to_path[i], screen_surface);
        if (!surface)
            success = false;
        game_media[i] = surface;
    }

    return success;
}

void update(void)
{

    /* Check if the space key has just been pressed  */
    bool space_pressed = false;
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    if (keyboard_state[SDL_SCANCODE_SPACE] && !space_down)
        space_pressed = true;
    space_down = keyboard_state[SDL_SCANCODE_SPACE];

    switch (state) {
    case STATE_MENU: {
        if (space_pressed) {
            state = STATE_PLAY;
            /* TODO: controls and game */
        } else {
            if (num_players == 2 && keyboard_state[SDL_SCANCODE_UP]) {
                num_players = 1;
            } else if (num_players == 1  && keyboard_state[SDL_SCANCODE_DOWN]) {
                num_players = 2;
            }
        }

        /* 'attract mode' update */
        game_update();
        break;
    }
    case STATE_PLAY: {
        /* TODO */
        break;
    }
    case STATE_GAME_OVER: {
        if (space_pressed) {
            /* TODO: reset to menu state */
        }
        break;
    }
    }
}

void draw(SDL_Surface *target_surface)
{
    game_draw(target_surface);

    switch (state) {
    case STATE_MENU: {
        SDL_Surface *menu_surface = game_media[MEDIA_MENU0 + num_players - 1];
        SDL_BlitSurface(menu_surface, NULL, target_surface, NULL);
        break;
    }
    case STATE_GAME_OVER: {
        SDL_Surface *over_surface = game_media[MEDIA_OVER];
        SDL_BlitSurface(over_surface, NULL, target_surface, NULL);
        break;
    }
    default:
        /* NOTE: ignore here */
        break;
    }
}

int main(int argc, char *argv[])
{
    (void) argc; (void) argv;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        goto err_init;
    }

    if(!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
        goto err_img;
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

    if (!load_media(screen_surface)) {
        fprintf(stderr, "Failed to load media files!");
        goto err_media;
    }

    game_reset();

    bool quit = false;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
        update();
        draw(screen_surface);
        SDL_UpdateWindowSurface(window);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;

err_media:
err_window:
    SDL_DestroyWindow(window);
err_img:
err_init:
    SDL_Quit();

    return EXIT_FAILURE;
}
