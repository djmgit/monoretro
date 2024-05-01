#ifndef PONG_H
#define PONG_H

#define PONG_GAME_STATE_MENU 0
#define PONG_GAME_STATE_RUN 1
#define PONG_GAME_STATE_GAME_OVER 2
#define WIN_SCORE 5

#define PADDLE_HEIGHT 16
#define PADDLE_WIDTH 4
#define PADDLE_SPEED 2

#define BALL_RADIUS 2
#define BALL_SPEED 1

typedef struct
{
    uint8_t posX;
    uint8_t posY;
    uint8_t width;
    uint8_t height;
    uint8_t speed;
} paddle_t;

typedef struct
{
    uint8_t posX;
    uint8_t posY;
    uint8_t radius;
    uint8_t speedX;
    uint8_t speedY;

} ball_t;

void pongSetup();
void pongUpdate();
void pongRender();

#endif
