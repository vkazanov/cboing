#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include <SDL.h>
#include <SDL_image.h>

#define HEIGHT 480
#define WIDTH 800
#define TITLE "CBoing!"

#define HALF_WIDTH (WIDTH / 2)
#define HALF_HEIGHT (HEIGHT / 2)

#define PLAYER_SPEED 3
#define MAX_AI_SPEED 3
#define BALL_INIT_SPEED 2

/* TODO: rename or rework */
/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) \
    ({                                                                  \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);            \
        (type *)( (char *)__mptr - offsetof(type,member) );             \
    })

int max(int a, int b) {
    return a > b ? a : b;
}

int min(int a, int b) {
    return a < b ? a : b;
}

typedef enum media_t {
    MEDIA_MENU0,
    MEDIA_MENU1,
    MEDIA_OVER,
    MEDIA_TABLE,
    MEDIA_BLANK,

    MEDIA_BAT00,
    MEDIA_BAT01,
    MEDIA_BAT02,

    MEDIA_BAT10,
    MEDIA_BAT11,
    MEDIA_BAT12,

    MEDIA_BALL,

    MEDIA_NUM,
} media_t;

char *media_to_path[MEDIA_NUM] = {
    [MEDIA_MENU0] = "images/menu0.png",
    [MEDIA_MENU1] = "images/menu1.png",
    [MEDIA_OVER] = "images/over.png",
    [MEDIA_TABLE] = "images/table.png",
    [MEDIA_BLANK] = "images/blank.png",

    [MEDIA_BAT00] = "images/bat00.png",
    [MEDIA_BAT01] = "images/bat01.png",
    [MEDIA_BAT02] = "images/bat02.png",
    [MEDIA_BAT10] = "images/bat10.png",
    [MEDIA_BAT11] = "images/bat11.png",
    [MEDIA_BAT12] = "images/bat12.png",

    [MEDIA_BALL] = "images/ball.png",
};

SDL_Surface *game_media[MEDIA_NUM];

enum {
    STATE_MENU = 1,
    STATE_PLAY = 2,
    STATE_GAME_OVER = 3,
} state = STATE_MENU;

typedef struct actor_t actor_t;
typedef void actor_update_func(actor_t *actor);
typedef void actor_draw_func(actor_t *actor, SDL_Surface *target_surface);

struct actor_t {
    media_t media;
    float x;
    float y;
    int h;
    int w;

    actor_update_func *update;
    actor_draw_func *draw;
};


void actor_draw(actor_t *actor, SDL_Surface *target_surface)
{
    SDL_Surface *source_surface = game_media[actor->media];
    SDL_Rect target_rect = {
        .x = actor->x,
        .y = actor->y,
    };
    SDL_BlitSurface(source_surface, NULL, target_surface, &target_rect);
}

typedef struct bat_t bat_t;

typedef int move_func (void);

struct bat_t {
    actor_t actor;

    int player;
    int score;
    int timer;

    move_func *move;
};

typedef struct ball_t ball_t;

typedef struct ball_t {
    actor_t actor;
    float dx;
    float dy;
    int speed;
} ball_t;

typedef struct game_t game_t;

typedef struct impact_t {
    actor_t actor;
} impact_t;

typedef struct game_t {
    bat_t bats[2];
    ball_t ball;
    impact_t *impact_list;
    int ai_offset;
} game_t;

game_t game;

bool ball_out(ball_t *ball);

void bat_update(actor_t *actor)
{
    bat_t *bat = container_of(actor, bat_t, actor);

    bat->timer -= 1;

    int y_movement = bat->move();

    actor->y = fmin(400 - actor->h / 2, fmax(0, actor->y + y_movement));

    int frame = 0;
    if (bat->timer > 0) {
        if (ball_out(&game.ball)) {
            frame = 2;
        } else {
            frame = 1;
        }
    }

    const int bat_to_media[] = {
        [0] = MEDIA_BAT00,
        [1] = MEDIA_BAT10,
    };
    bat->actor.media = bat_to_media[bat->player] + frame;
}


void bat_init(bat_t *bat, int player, move_func *move)
{
    bat->move = move;

    bat->actor.update = bat_update;
    bat->actor.draw = actor_draw;

    bat->actor.media = MEDIA_BLANK;

    /* TODO: this data is static, can be inlined in all places */
    const int bat_half_width = game_media[MEDIA_BAT00]->w / 2;
    const int bat_half_height = game_media[MEDIA_BAT00]->h / 2;

    bat->actor.x = (player == 0) ? (40 - bat_half_width): (760 - bat_half_width);
    bat->actor.y = HALF_HEIGHT - bat_half_height;
    bat->actor.w = game_media[MEDIA_BAT00]->w;
    bat->actor.h = game_media[MEDIA_BAT00]->h;

    bat->player = player;
    bat->score = 0;

    /* TODO: move func */

    bat->timer = 0;

}

void normalise(float *dx, float *dy)
{
    float length = hypotf(*dx, *dy);
    *dx = *dx / length;
    *dy = *dy / length;
}

void ball_update(actor_t *actor)
{
    ball_t *ball = container_of(actor, ball_t, actor);

    for (int i = 0; i < ball->speed; ++i) {
        float original_x = actor->x;

        actor->x += ball->dx;
        actor->y += ball->dy;

        /* bounce a bat */
        if (abs((int)actor->x - HALF_WIDTH) >= 344 && abs((int)original_x - HALF_WIDTH) < 344) {

            bat_t *bat = NULL;
            int new_dir_x;
            if (actor->x < HALF_WIDTH) {
                new_dir_x = 1;
                bat = &game.bats[0];
            } else {
                new_dir_x = -1;
                bat = &game.bats[1];
            }

            float difference_y = (actor->y + actor->h / 2) - (bat->actor.y + bat->actor.h / 2);

            if (difference_y > -64 && difference_y < 64) {
                /* collision happenned, find new direction vector */

                ball->dx = -ball->dx;

                ball->dy += difference_y / 128;

                ball->dy = fmin(fmax(ball->dy, -1), 1);

                normalise(&ball->dx, &ball->dy);

                ball->speed += 1;

                /* TODO: fix ai offset */

                bat->timer = 10;

                /* TODO: sound */
                /* TODO: impacts */
            }
        }

        /* check if hits arena top or bottom */
        if (fabs(actor->y - HALF_HEIGHT) > 220) {
            ball->dy = -ball->dy;
            actor->y += ball->dy;
        }

    }

}

void ball_init(ball_t *ball, int dx)
{
    ball->actor.media = MEDIA_BALL;
    ball->actor.update = ball_update;
    ball->actor.draw = actor_draw;
    ball->actor.x = HALF_WIDTH;
    ball->actor.y = HALF_HEIGHT;
    ball->actor.w = game_media[MEDIA_BALL]->w;
    ball->actor.h = game_media[MEDIA_BALL]->h;

    ball->dx = dx;
    ball->dy = 0;
    ball->speed = BALL_INIT_SPEED;
}

bool ball_out(ball_t *ball)
{
    return ball->actor.x < 0 || ball->actor.x > WIDTH;
}

int p1_move(void)
{
    int move = 0;
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    if (keyboard_state[SDL_SCANCODE_Z] || keyboard_state[SDL_SCANCODE_DOWN])
        move = PLAYER_SPEED;
    else if (keyboard_state[SDL_SCANCODE_A] || keyboard_state[SDL_SCANCODE_UP])
        move = -PLAYER_SPEED;
    return move;
}

int p2_move(void)
{
    int move = 0;
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    if (keyboard_state[SDL_SCANCODE_M])
        move = PLAYER_SPEED;
    else if (keyboard_state[SDL_SCANCODE_K])
        move = -PLAYER_SPEED;
    return move;
}

void game_reset(void)
{
    bat_init(&game.bats[0], 0, p1_move);
    bat_init(&game.bats[1], 1, p2_move);

    ball_init(&game.ball, -1);


    /* TODO: impacts */

    game.ai_offset = 0;
}

void game_update(void)
{
    /* TODO: it should be possible to just register actors instead of manually
     * listing everything */
    game.bats[0].actor.update(&game.bats[0].actor);
    game.bats[1].actor.update(&game.bats[1].actor);
    game.ball.actor.update(&game.ball.actor);

    /* TODO: update impacts*/

    /* TODO: drop expired impacts*/

    /* ball out */
    if (ball_out(&game.ball)) {
        int scoring_player = game.ball.actor.x < WIDTH / 2 ? 1 : 0;
        int losing_player = 1 - scoring_player;

        if (game.bats[losing_player].timer < 0) {
            game.bats[scoring_player].score += 1;
            game.bats[losing_player].timer = 20;
        } else if (game.bats[losing_player].timer == 0) {
            int direction = losing_player == 0 ? -1 : 1;
            ball_init(&game.ball, direction);
        }
    }
}

void game_draw(SDL_Surface *target_surface)
{
    /* background */
    SDL_Surface *table_surface = game_media[MEDIA_TABLE];
    SDL_BlitSurface(table_surface, NULL, target_surface, NULL);

    /* 'just scored' */
    /* TODO: */

    /* TODO: should be possible to just draw known actors, same as above */
    game.bats[0].actor.draw(&game.bats[0].actor, target_surface);
    game.bats[1].actor.draw(&game.bats[1].actor, target_surface);
    game.ball.actor.draw(&game.ball.actor, target_surface);

    /* impacts */
    /* TODO: */

    /* scores */
    /* TODO: */

}

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

int num_players = 1;

bool space_down = false;

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
            game_reset();
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
        if (max(game.bats[0].score, game.bats[1].score) > 9) {
            state = STATE_GAME_OVER;
        } else {
            game_update();
        }
        break;
    }
    case STATE_GAME_OVER: {
        if (space_pressed) {
            state = STATE_MENU;
            num_players = 1;

            game_reset();
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
    default: break;
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
        fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
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
