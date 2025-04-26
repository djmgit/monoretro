#include "common.h"
#include "monoretro.h"
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

uint8_t BREAK_GAME_STATE = BREAK_GAME_STATE_MENU;

brick_t bricks[NUM_BRICKS];

break_paddle_t paddle = {
    .posX = SCREEN_WIDTH / 2 - BREAK_PADDLE_WIDTH / 2,
    .posY = PADDLE_POSY,
    .width = BREAK_PADDLE_WIDTH,
    .height = BREAK_PADDLE_HEIGHT,
    .speed = BREAK_PADDLE_SPEED
};

break_ball_t breakBall = {
    .posX = SCREEN_WIDTH / 2,
    .posY = SCREEN_HEIGHT / 2 + 10,
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

int breakScore = 0;
uint8_t bricksHit = 0;

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
    int leftVal = isLeftPressed();
    int rightVal = isRightPressed();
    int actionVal = digitalRead(FIRE);

    if (BREAK_GAME_STATE == BREAK_GAME_STATE_RUN) {
        if (leftVal == 0 && (paddle.posX > 0))
        {
            paddle.posX -= paddle.speed;
        }
        if (rightVal == 0 && paddle.posX + paddle.width < SCREEN_WIDTH)
        {
            paddle.posX += paddle.speed;
        }
    } else if (BREAK_GAME_STATE == BREAK_GAME_STATE_MENU) {
        if (actionVal == 0) {
            BREAK_GAME_STATE = BREAK_GAME_STATE_RUN;
        }
    } else if (BREAK_GAME_STATE == BREAK_GAME_STATE_GAME_OVER) {
        if (actionVal == 0) {
            breakStartGame();
            BREAK_GAME_STATE = BREAK_GAME_STATE_RUN;
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
        if ((ballX + radius > brick.posX) && (ballX - radius < brick.posX + brick.width) && (ballY - radius < brick.posY + brick.height) && (ballY + radius > brick.posY + brick.height)) {
            bricks[i].hit = 1;
            breakBall.speedY *= -1;
            bricksHit++;
            breakScore += BRICK_HIT_SCORE;
            continue;
        }

        // ball hits brom left
        if ((ballY - radius < brick.posY + brick.height) && (ballY + radius > brick.posY) && (ballX - radius < brick.posX + brick.width) && (ballX + radius > brick.posX + brick.width)) {
            bricks[i].hit = 1;
            breakBall.speedX *= -1;
            bricksHit++;
            breakScore += BRICK_HIT_SCORE;
            continue;
        }

        // ball hits from right
        if ((ballY - radius < brick.posY + brick.height) && (ballY + radius > brick.posY) && (ballX + radius > brick.posX) && (ballX - radius < brick.posX)) {
            bricks[i].hit = 1;
            breakBall.speedX *= -1;
            bricksHit++;
            breakScore += BRICK_HIT_SCORE;
            continue;
        }

        // ball hits from up
        if ((ballX + radius > brick.posX) && (ballX - radius < brick.posX + brick.width) && (ballY + radius > brick.posY) && (ballY - radius < brick.posY)) {
            bricks[i].hit = 1;
            breakBall.speedY *= -1;
            bricksHit++;
            breakScore += BRICK_HIT_SCORE;
            continue;
        }

    }
}

void checkGroundHit() {
    if (breakBall.posY + breakBall.radius > SCREEN_HEIGHT) {
        breakGameOver();
    }
}

void breakGameOver() {
    BREAK_GAME_STATE = BREAK_GAME_STATE_GAME_OVER;
}

void checkAllHit() {
    if (bricksHit != NUM_BRICKS) {
        return;
    }

    breakResetGame();
}

void resetBricks() {
    for (size_t i = 0; i < NUM_BRICKS; i++) {
        bricks[i].hit = 0;
    }
}


void drawBrick(brick_t brick) {
    monoDisplay.fillRect(brick.posX, brick.posY, brick.width, brick.height, SSD1306_INVERSE);
}

void breakResetGame() {
    bricksHit = 0;
    breakBall.posX = SCREEN_WIDTH / 2 - BREAK_PADDLE_WIDTH / 2;
    breakBall.posY = SCREEN_HEIGHT / 2 + 10;
    paddle.posX = SCREEN_WIDTH / 2;
    resetBricks();
    
}

void breakStartGame() {
    breakBall.speedX = BREAK_BALL_SPEED;
    breakBall.speedY = BREAK_BALL_SPEED;
    breakScore = 0;
    breakResetGame();
    generateBricks();
}

void breakUpdate() {
    breakProcessInput();
    if (BREAK_GAME_STATE == BREAK_GAME_STATE_RUN) {
        updateBreakBallPosition();
        checkBoundaryHit();
        checkPaddleHit();
        checkBrickHit();
        checkAllHit();
        checkGroundHit();
    }
}

void breakDrawSpashScreen() {
    monoDisplay.setTextSize(2);
    monoDisplay.setTextColor(WHITE);
    monoDisplay.setCursor(40, 20);
    monoDisplay.println("BRICKS");
    monoDisplay.setTextSize(1);
    monoDisplay.setCursor(20, 50);
    monoDisplay.println("Press fire ...");
}

void breakGameOverScreen() {
    monoDisplay.setTextSize(2);
    monoDisplay.setTextColor(WHITE);
    monoDisplay.setCursor(0, 10);
    monoDisplay.println("GAME OVER\n");
    monoDisplay.setTextSize(1);
    monoDisplay.setCursor(30, 30);
    monoDisplay.printf("Score: %d\n", breakScore);
    monoDisplay.setCursor(20, 50);
    monoDisplay.printf("Press fire ...");
}

void breakRender() {
    monoDisplay.clearDisplay();
    if (BREAK_GAME_STATE == BREAK_GAME_STATE_RUN) {
        for (size_t i = 0; i < NUM_BRICKS; i++) {
            if (bricks[i].hit == 0) {
                drawBrick(bricks[i]);
            }
        }
        monoDisplay.fillRect(paddle.posX, paddle.posY, paddle.width, paddle.height, SSD1306_INVERSE);
        monoDisplay.fillCircle(breakBall.posX, breakBall.posY, breakBall.radius, SSD1306_INVERSE);
    } else if (BREAK_GAME_STATE == BREAK_GAME_STATE_MENU) {
        breakDrawSpashScreen();
    } else if (BREAK_GAME_STATE == BREAK_GAME_STATE_GAME_OVER) {
        breakGameOverScreen();
    }
    monoDisplay.display();
}

void breakSetup()
{
    printf("test test.....\n");
    BREAK_GAME_STATE = BREAK_GAME_STATE_MENU;
    breakStartGame();
}

