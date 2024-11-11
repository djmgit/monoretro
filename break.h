#ifndef BREAK_H
#define BREAK_H

#define BREAK_GAME_STATE_MENU 0
#define BREAK_GAME_STATE_RUN 1
#define BREAK_GAME_STATE_GAME_OVER 2
#define BREAK_WIN_SCORE 5

#define BREAK_PADDLE_WIDTH 16
#define BREAK_PADDLE_HEIGHT 4
#define BREAK_PADDLE_SPEED 4
#define BREAK_BALL_SPEED 1
#define BRICK_WIDTH 8
#define BRICK_HEIGHT 4
#define NUM_BRICKS 60
#define BRICK_HIT_SCORE 2
#define PADDLE_POSY 60

void breakSetup();

void breakUpdate();

void breakRender();

typedef struct
{
    uint8_t posX;
    uint8_t posY;
    uint8_t width;
    uint8_t height;
    uint8_t hit;
} brick_t;

typedef struct {
    uint8_t posX;
    uint8_t posY;
    uint8_t radius;
    uint8_t speedX;
    uint8_t speedY;
} break_ball_t;

typedef struct {
    uint8_t posX;
    uint8_t posY;
    uint8_t width;
    uint8_t height;
    uint8_t speed;
} break_paddle_t;

#endif
