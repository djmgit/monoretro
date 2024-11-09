#include "common.h"
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "space_invaders.h"
#include "break.h"

uint8_t BREAK_GAME_STATE = BREAK_GAME_STATE_RUN;

brick_t bricks[NUM_BRICKS];

break_paddle_t paddle = {
    .posX = SCREEN_WIDTH / 2,
    .posY = 60,
    .width = BREAK_PADDLE_WIDTH,
    .height = BREAK_PADDLE_HEIGHT,
    .speed = BREAK_PADDLE_SPEED
};

break_ball_t breakBall = {
    .posX = SCREEN_WIDTH / 2,
    .posY = SCREEN_HEIGHT / 2,
    .radius = 2,
    .speedX = BREAK_BALL_SPEED,
    .speedY = BREAK_BALL_SPEED
};

brick_t spawnBrick(uint8_t posX, uint8_t posY, uint8_t width, uint8_t height) {
    return (brick_t) {
        .posX = posX,
        .posY = posY,
        .width = width,
        .height = height,
        .hit = 0
    };
}

void generateBricks() {
    uint8_t posX = 4;
    uint8_t posY = 4;

    uint8_t count = 0;
    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 12; j++) {
            bricks[count++] = spawnBrick(posX, posY, BRICK_WIDTH, BRICK_HEIGHT);
            printf("%d\n", posX);
            posX += BRICK_WIDTH + 2;
        }
        posX = 4;
        posY += BRICK_HEIGHT + 2;
    }
}

void breakProcessInput()
{
    int leftVal = digitalRead(LEFT);
    int rightVal = digitalRead(RIGHT);

    if (BREAK_GAME_STATE == BREAK_GAME_STATE_RUN) {
        if (leftVal == 0 && (paddle.posX > 0))
        {
            paddle.posX -= paddle.speed;
        }
        if (rightVal == 0 && paddle.posX + paddle.width < SCREEN_WIDTH)
        {
            paddle.posX += paddle.speed;
        }
    }
}

void updateBreakBallPosition() {
    breakBall.posX += breakBall.speedX;
    breakBall.posY += breakBall.speedY;
}

void checkBoundaryHit() {
    if ((breakBall.posX - breakBall.radius < 0) || (breakBall.posX + breakBall.radius > SCREEN_WIDTH)) {
        breakBall.speedX *= -1;
    }
    if (breakBall.posY - breakBall.radius < 0) {
        breakBall.speedY *= -1;
    }
}

void checkPaddleHit() {
    if ((breakBall.posX + breakBall.radius > paddle.posX) && (breakBall.posX - breakBall.radius < paddle.posX + paddle.width) && (breakBall.posY + breakBall.radius > paddle.posY)) {
        breakBall.speedY *= -1;
    }
}

void checkBrickHit() {
    uint8_t ballX = breakBall.posX;
    uint8_t ballY = breakBall.posY;
    uint8_t radius = breakBall.radius;
    for (size_t i = 0; i < NUM_BRICKS; i++) {
        brick_t brick = bricks[i];
        if (brick.hit == 1) {
            continue;
        }

        // ball hits from below the brick
        if ((ballX + radius > brick.posX) && (ballX - radius < brick.posX + brick.width) && (ballY - radius < brick.posY + brick.height)) {
            brick.hit = 1;
            bricks[i].hit = 1;
            breakBall.speedY *= -1;
            break;
        }
    }
}


void drawBrick(brick_t brick) {
    monoDisplay.fillRect(brick.posX, brick.posY, brick.width, brick.height, SSD1306_INVERSE);
}

void breakStartGame() {
    generateBricks();
}

void breakUpdate() {
    breakProcessInput();
    updateBreakBallPosition();
    checkBoundaryHit();
    checkPaddleHit();
    checkBrickHit();
}

void breakRender() {
    monoDisplay.clearDisplay();
    //drawBrick((brick_t){.posX = 20, .posY = 20, .width = 8, .height = 4, .hit = 0});
    //drawBrick(bricks[0]);
    for (size_t i = 0; i < NUM_BRICKS; i++) {
        if (bricks[i].hit == 0) {
            drawBrick(bricks[i]);
        }
    }
    monoDisplay.fillRect(paddle.posX, paddle.posY, paddle.width, paddle.height, SSD1306_INVERSE);
    monoDisplay.fillCircle(breakBall.posX, breakBall.posY, breakBall.radius, SSD1306_INVERSE);
    monoDisplay.display();

}

void breakSetup()
{
    printf("test test.....\n");
    breakStartGame();
}

