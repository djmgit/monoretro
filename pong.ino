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
#include "pong.h"

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

uint8_t PONG_GAME_STATE = PONG_GAME_STATE_MENU;
int scorePlayer = 0;
int scoreOpp = 0;
int playerWon = 0;


// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 pongDisplay(SCREEN_WIDTH, SCREEN_HEIGHT);

paddle_t player = {
    .posX = SCREEN_WIDTH - PADDLE_WIDTH,
    .posY = SCREEN_HEIGHT / 2,
    .width = PADDLE_WIDTH,
    .height = PADDLE_HEIGHT,
    .speed = PADDLE_SPEED
};

paddle_t opp = {
    .posX = 0,
    .posY = SCREEN_HEIGHT / 2,
    .width = PADDLE_WIDTH,
    .height = PADDLE_HEIGHT,
    .speed = PADDLE_SPEED
};

ball_t ball = {
    .posX = SCREEN_WIDTH / 2,
    .posY = SCREEN_HEIGHT / 2,
    .radius = BALL_RADIUS,
    .speedX = BALL_SPEED,
    .speedY = BALL_SPEED
};

void pongSetup()
{
    Serial.begin(115200);
    srand(time(NULL));

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!pongDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    //pongDisplay.display();
    // Clear the buffer
    pongDisplay.clearDisplay();
    pinMode(down, INPUT_PULLUP);
    pinMode(up, INPUT_PULLUP);
    pinMode(action, INPUT_PULLUP);

    pongStartGame();
}

void displayScore() {
    pongDisplay.setTextSize(1);
    pongDisplay.setTextColor(WHITE);
    pongDisplay.setCursor(20, 5);
    pongDisplay.printf("%d", scoreOpp);
    pongDisplay.setCursor(84, 5);
    pongDisplay.printf("%d", scorePlayer);
}

void updateOppPosition() {
    if (opp.posY > ball.posY && opp.posY > 0) {
        opp.posY -= opp.speed;
    }

    if (opp.posY < ball.posY && opp.posY + opp.height < SCREEN_HEIGHT) {
        opp.posY += opp.speed;
    }
}

void resetGame() {
    uint8_t xDir = rand() / (RAND_MAX / 2 + 1);
    ball.speedX = xDir == 0 ? ball.speedX * -1 : ball.speedX;
    uint8_t yDir = rand() / (RAND_MAX / 2 + 1);
    ball.speedY = yDir == 1 ? ball.speedY * -1 : ball.speedY;

    player.posY = SCREEN_HEIGHT / 2;
    opp.posY = SCREEN_HEIGHT / 2;
    ball.posX = SCREEN_WIDTH / 2;
    ball.posY = SCREEN_HEIGHT / 2;
}

void pongStartGame() {
    scorePlayer = 0;
    scoreOpp = 0;
    playerWon = 0;
    ball.speedX = BALL_SPEED;
    ball.speedY = BALL_SPEED;
    resetGame();
}

void checkWinner() {
    if (scorePlayer == WIN_SCORE) {
        playerWon = 1;
        PONG_GAME_STATE = PONG_GAME_STATE_GAME_OVER;
    } else if (scoreOpp == WIN_SCORE) {
        playerWon = 0;
        PONG_GAME_STATE = PONG_GAME_STATE_GAME_OVER;
    } else {
        resetGame();
    }
}

void checkBallHit() {
    if ((ball.posY -  ball.radius < 0) || (ball.posY + ball.radius > SCREEN_HEIGHT)) {
        ball.speedY *= -1;
    }

    if (ball.posX - ball.radius < 0) {
        scorePlayer += 1;
        if (scorePlayer > 2) {
            ball.speedX = scorePlayer - 1;
            ball.speedY = scorePlayer - 1;
        }
        checkWinner();
    }

    if (ball.posX + ball.radius > SCREEN_WIDTH) {
        scoreOpp += 1;
        checkWinner();
    }

    if (ball.posY > player.posY && ball.posY < player.posY + player.height && ball.posX + ball.radius > player.posX) {
        ball.speedX *= -1;
    }

    if (ball.posY > opp.posY && ball.posY < opp.posY + player.height && ball.posX - ball.radius < opp.posX + opp.width) {
        ball.speedX *= -1;
    }

}

void updateBallPositions() {
    ball.posX += ball.speedX;
    ball.posY += ball.speedY;
}

static void pongProcessInput()
{
    int upVal = digitalRead(up);
    int downVal = digitalRead(down);
    int actionVal = digitalRead(action);
    // Serial.printf("%d  %d   %d\n", leftVal, rightVal, fireVal);
    if (PONG_GAME_STATE == PONG_GAME_STATE_RUN) {
        if (upVal == 0 && (player.posY > 0))
        {
            player.posY -= player.speed;
        }
        if (downVal == 0 && player.posY + player.height < SCREEN_HEIGHT)
        {
            player.posY += player.speed;
        }
    } else if (PONG_GAME_STATE == PONG_GAME_STATE_MENU) {
        if (actionVal == 0) {
            PONG_GAME_STATE = PONG_GAME_STATE_RUN;
        }
    } else if (PONG_GAME_STATE == PONG_GAME_STATE_GAME_OVER) {
        if (actionVal == 0) {
            pongStartGame();
            PONG_GAME_STATE = PONG_GAME_STATE_RUN;
        }
    }
}

void drawGround() {
    pongDisplay.drawFastHLine(0, 0, SCREEN_WIDTH, SSD1306_INVERSE);
    pongDisplay.drawFastHLine(0, SCREEN_HEIGHT-1, SCREEN_WIDTH, SSD1306_INVERSE);
    pongDisplay.drawFastVLine(SCREEN_WIDTH / 2, 0, SCREEN_HEIGHT, SSD1306_INVERSE);
    pongDisplay.drawCircle(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 10, SSD1306_INVERSE);
}

void drawSplashScreen() {
    pongDisplay.setTextSize(2);
    pongDisplay.setTextColor(WHITE);
    pongDisplay.setCursor(40, 20);
    pongDisplay.println("PONG");
    pongDisplay.setTextSize(1);
    pongDisplay.setCursor(20, 50);
    pongDisplay.println("Press fire ...");
}

void pongGameOver() {
    pongDisplay.setTextSize(2);
    pongDisplay.setTextColor(WHITE);
    pongDisplay.setCursor(10, 20);
    if (playerWon == 1) {
        pongDisplay.println("You win!");
    } else if (playerWon == 0) {
        pongDisplay.println("You lose!");
    }
}

void pongRender() {
    pongDisplay.clearDisplay();
    if (PONG_GAME_STATE == PONG_GAME_STATE_RUN) {
        drawGround();
        pongDisplay.fillRect(player.posX, player.posY, player.width, player.height, SSD1306_INVERSE);
        pongDisplay.fillRect(opp.posX, opp.posY, opp.width, opp.height, SSD1306_INVERSE);
        pongDisplay.fillCircle(ball.posX, ball.posY, ball.radius, SSD1306_INVERSE);
        displayScore();
    } else if (PONG_GAME_STATE == PONG_GAME_STATE_MENU) {
        drawSplashScreen();
    } else if (PONG_GAME_STATE == PONG_GAME_STATE_GAME_OVER) {
        pongGameOver();
    }
    pongDisplay.display();
}

void pongUpdate() {
    pongProcessInput();

    if (PONG_GAME_STATE == PONG_GAME_STATE_RUN) {
        updateOppPosition();
        updateBallPositions();
        checkBallHit();
    }
}



