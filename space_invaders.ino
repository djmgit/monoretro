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

#define GAME_STATE_MENU 0
#define GAME_STATE_RUN 1
#define GAME_STATE_GAME_OVER 2
#define MAX_LIVES 3

#define NUMFLAKES 10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT 16
#define LOGO_WIDTH 16

uint8_t paddleX = 0;
uint8_t paddleY = 62;
uint8_t fired = 0;

struct bulletNode
{
    bulletNode *next;
    bullet_t bullet;
};

typedef struct bulletNode bulletNode_t;

const uint8_t ALIEN_WIDTH = 8;
const uint8_t ALIEN_HEIGHT = 4;
const uint8_t NUM_ALIEN_ROWS = 4;
uint8_t ALIEN_SPEED = 1;
const uint8_t NUM_ALIENS = 28;

const uint8_t TANK_WIDTH = 16;
const uint8_t TANK_HEIGHT = 2;
const uint8_t TANK_SPEED = 2;

const uint8_t BULLET_HEIGHT = 8;
const uint8_t BULLET_WIDTH = 2;

uint8_t GAME_STATE = GAME_STATE_MENU;

const uint8_t ALIEN_BITMAP[4][8] = {
    {0, 0, 1, 1, 1, 1, 0, 0},
    {1, 1, 0, 1, 1, 0, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1},
    {0, 0, 1, 0, 0, 1, 0, 0}};

const uint8_t TANK_BITMAP[4][8] = {
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 0, 0, 1, 1, 0, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1}};

const uint8_t BULLET_BITMAP[8][2] = {
    {0, 1},
    {1, 0},
    {0, 1},
    {1, 0},
    {0, 1},
    {1, 0},
    {0, 1},
    {1, 0}};

const uint16_t ALIEN_BULLET_SPAWN_TIME = 1000.0;
float alienBulletTimeRemaining = 0.0;



int score = 0;

uint8_t lives = 4;

player_t tank = {
    .posX = SCREEN_WIDTH / 2,
    .posY = 60,
    .width = 8,
    .height = 2,
    .alive = 1,
    .speed = 2};

uint8_t ALIEN_DIRECTION = -1;

alien_t aliens[28];

bulletNode_t *tankBulletList;
bulletNode_t *alienBulletList;

void resetTankPosition()
{
    tank.posX = SCREEN_WIDTH / 2;
    tank.posY = 60;
}

void starGame() {
    lives = 4;
    //memset(aliens, 0, NUM_ALIENS);
    resetTankPosition();
    score = 0;
    ALIEN_DIRECTION = -1;
    ALIEN_SPEED = 1;
    loadAliens();
}

void newWave() {
    ALIEN_SPEED++;
    ALIEN_DIRECTION = -1;
    loadAliens();
}

void spawnTankBullet(uint8_t posX, uint8_t posY)
{

    uint8_t chance = rand() / (RAND_MAX / 10);
    if (chance > 6) {
        return;
    }
    bullet_t bullet = {
        .posX = posX,
        .posY = posY,
        .speed = 1,
        .alive = 1};
    bulletNode_t *bNode = (bulletNode_t *)malloc(sizeof(bulletNode_t));
    bNode->bullet = bullet;
    bNode->next = NULL;
    if (tankBulletList == NULL)
    {
        tankBulletList = bNode;
        return;
    }
    bulletNode_t *tHead = tankBulletList;
    while (tHead->next != NULL)
    {
        tHead = tHead->next;
    }
    tHead->next = bNode;
}

void spawnAlienBullet(uint8_t posX, uint8_t posY)
{
    bullet_t bullet = {
        .posX = posX,
        .posY = posY,
        .speed = 1,
        .alive = 1};
    bulletNode_t *bNode = (bulletNode_t *)malloc(sizeof(bulletNode_t));
    bNode->bullet = bullet;
    bNode->next = NULL;
    if (alienBulletList == NULL)
    {
        alienBulletList = bNode;
        return;
    }
    bulletNode_t *tHead = alienBulletList;
    while (tHead->next != NULL)
    {
        tHead = tHead->next;
    }
    tHead->next = bNode;
}

void loadAliens()
{
    uint8_t posX = 24;
    uint8_t posY = 8;

    uint8_t alientIndex = 0;
    for (size_t row = 1; row <= NUM_ALIEN_ROWS; row++)
    {
        while (posX < SCREEN_WIDTH)
        {
            aliens[alientIndex] = spawnAlien(posX, posY, 1);
            posX += 16;
            alientIndex += 1;
        }
        posX = 24;
        posY += 8;
    }
}

alien_t spawnAlien(uint8_t posX, uint8_t posY, uint8_t alienType)
{
    return (alien_t){
        .posX = posX,
        .posY = posY,
        .width = ALIEN_WIDTH,
        .height = ALIEN_HEIGHT,
        .alienType = alienType,
        .alive = 1};
}

void drawAlien(uint8_t posX, uint8_t posY)
{
    for (size_t row = 0; row < 4; row++)
    {
        for (size_t col = 0; col < 8; col++)
        {
            if (ALIEN_BITMAP[row][col] == 1)
            {
                monoDisplay.drawPixel(posX + col, posY + row, SSD1306_INVERSE);
            }
        }
    }
}

void drawTank(uint8_t posX, uint8_t posY)
{
    for (size_t row = 0; row < 4; row++)
    {
        for (size_t col = 0; col < 8; col++)
        {
            if (TANK_BITMAP[row][col] == 1)
            {
                monoDisplay.drawPixel(posX + col, posY + row, SSD1306_INVERSE);
            }
        }
    }
}

void drawBullet(uint8_t posX, uint8_t posY)
{
    for (size_t row = 0; row < 8; row++)
    {
        for (size_t col = 0; col < 2; col++)
        {
            if (BULLET_BITMAP[row][col] == 1)
            {
                monoDisplay.drawPixel(posX + col, posY + row, SSD1306_INVERSE);
            }
        }
    }
}

void displaySplash() {
    monoDisplay.setTextSize(2);
    monoDisplay.setTextColor(WHITE);
    monoDisplay.setCursor(40, 10);
    monoDisplay.println("SPACE");
    monoDisplay.setCursor(20, 30);
    monoDisplay.println("INVADERS");
    monoDisplay.setTextSize(1);
    monoDisplay.setCursor(40, 50);
    monoDisplay.println("Press fire");
}

void gameOver() {
    monoDisplay.setTextSize(2);
    monoDisplay.setTextColor(WHITE);
    monoDisplay.setCursor(0, 10);
    monoDisplay.println("GAME OVER!");
    monoDisplay.setTextSize(1);
    monoDisplay.setCursor(30, 30);
    monoDisplay.printf("Score: %d", score);

}

void space_invaders_render()
{
    monoDisplay.clearDisplay();
    if (GAME_STATE == GAME_STATE_RUN) {
        for (size_t alienIndex = 0; alienIndex < 28; alienIndex++)
        {
            if (aliens[alienIndex].alive == 1)
            {
                drawAlien(aliens[alienIndex].posX, aliens[alienIndex].posY);
            }
        }
        if (tank.alive == 1)
        {
            drawTank(tank.posX, tank.posY);
        }
        bulletNode_t *tHead = tankBulletList;
        while (tHead != NULL)
        {
            if (tHead->bullet.alive == 1)
            {
                drawBullet(tHead->bullet.posX, tHead->bullet.posY);
            }
            tHead = tHead->next;
        }

        tHead = alienBulletList;
        while (tHead != NULL)
        {
            if (tHead->bullet.alive == 1)
            {
                drawBullet(tHead->bullet.posX, tHead->bullet.posY);
            }
            tHead = tHead->next;
        }
    } else if (GAME_STATE == GAME_STATE_MENU) {
        displaySplash();
    } else if (GAME_STATE == GAME_STATE_GAME_OVER) {
        gameOver();
    }
    monoDisplay.display();
}

uint8_t getRandomAliveAlien()
{
    uint8_t aliveAliens[28];
    uint8_t count = 0;
    for (size_t i = 0; i < NUM_ALIENS; i++)
    {
        if (aliens[i].alive == 1)
        {
            aliveAliens[count++] = i;
        }
    }
    uint8_t rAlien = rand() / (RAND_MAX / count + 1);
    return aliveAliens[rAlien];
}

void cleanTankBullets()
{
    bulletNode_t *tHead = tankBulletList;
    while (tHead != NULL && tHead->bullet.alive == 0)
    {
        bulletNode_t *temp = tHead;
        tHead = tHead->next;
        free(temp);
    }
    tankBulletList = tHead;

    bulletNode_t *prev = tankBulletList;
    while (prev != NULL)
    {
        bulletNode_t *tHead = prev->next;
        while (tHead != NULL && tHead->bullet.alive == 0)
        {
            bulletNode_t *temp = tHead;
            tHead = tHead->next;
            free(temp);
        }
        prev->next = tHead;
        prev = prev->next;
    }
}

void cleanAlienBullets()
{
    bulletNode_t *tHead = alienBulletList;
    while (tHead != NULL && tHead->bullet.alive == 0)
    {
        bulletNode_t *temp = tHead;
        tHead = tHead->next;
        free(temp);
    }
    alienBulletList = tHead;

    bulletNode_t *prev = alienBulletList;
    while (prev != NULL)
    {
        bulletNode_t *tHead = prev->next;
        while (tHead != NULL && tHead->bullet.alive == 0)
        {
            bulletNode_t *temp = tHead;
            tHead = tHead->next;
            free(temp);
        }
        prev->next = tHead;
        prev = prev->next;
    }
}

void spawnRandomBullet()
{
    alienBulletTimeRemaining -= (float)(1000 / 60);
    if (alienBulletTimeRemaining <= 0)
    {
        alienBulletTimeRemaining = ALIEN_BULLET_SPAWN_TIME;
        alien_t rAlien = aliens[getRandomAliveAlien()];
        spawnAlienBullet(rAlien.posX + (ALIEN_WIDTH / 2), rAlien.posY + (ALIEN_HEIGHT / 2));
    }
}

void updateTankBullets()
{
    bulletNode_t *tHead = tankBulletList;
    while (tHead != NULL)
    {
        if (tHead->bullet.alive == 1)
        {
            tHead->bullet.posY -= tHead->bullet.speed;
            if (tHead->bullet.posY < 0 || tHead->bullet.posY > SCREEN_HEIGHT)
            {
                tHead->bullet.alive = 0;
            }
        }
        tHead = tHead->next;
    }
}

void updateAlienBullets()
{
    bulletNode_t *tHead = alienBulletList;
    while (tHead != NULL)
    {
        if (tHead->bullet.alive == 1)
        {
            tHead->bullet.posY += tHead->bullet.speed;
            if (tHead->bullet.posY + BULLET_HEIGHT > SCREEN_HEIGHT)
            {
                tHead->bullet.alive = 0;
            }
        }
        tHead = tHead->next;
    }
}

void updateAlienPositions()
{
    int updateAlienDirection = 0;

    for (size_t alienIndex = 0; alienIndex < NUM_ALIENS; alienIndex++)
    {
        if (aliens[alienIndex].alive == 1)
        {
            aliens[alienIndex].posX += (ALIEN_SPEED * ALIEN_DIRECTION);
            if (aliens[alienIndex].posX >= SCREEN_WIDTH || aliens[alienIndex].posX + ALIEN_WIDTH >= SCREEN_WIDTH)
            {
                updateAlienDirection = 1;
            }
        }
    }

    ALIEN_DIRECTION = updateAlienDirection == 1 ? ALIEN_DIRECTION * -1 : ALIEN_DIRECTION;

    uint8_t moveDown = 1;
    if (moveDown == 1 && updateAlienDirection == 1 && ALIEN_DIRECTION == 1)
    {
        if (updateAlienDirection == 1)
        {
            for (size_t i = 0; i < NUM_ALIENS; i++)
            {
                if (aliens[i].alive == 1)
                {
                    aliens[i].posY += 1;
                    if (aliens[i].posY + ALIEN_WIDTH > tank.posY) {
                        GAME_STATE = GAME_STATE_GAME_OVER;
                        break;
                    }
                }
            }
        }
    }
}

void checkAlienHit()
{
    bulletNode_t *tHead = tankBulletList;
    while (tHead != NULL)
    {
        uint8_t aliveCount = 0;
        for (size_t i = 0; i < NUM_ALIENS; i++)
        {
            if (aliens[i].alive == 0)
            {
                continue;
            }
            double dist = getDistance(tHead->bullet.posX, tHead->bullet.posY, aliens[i].posX + (ALIEN_WIDTH / 2), aliens[i].posY + (ALIEN_HEIGHT / 2));
            if (dist <= 4.0)
            {
                tHead->bullet.alive = 0;
                aliens[i].alive = 0;
                score += 10;
            }
            if (aliens[i].alive == 1) {
                aliveCount++;
            }
        }
        if (aliveCount == 0) {
            newWave();
            break;
        }
        tHead = tHead->next;
    }
}

void checkTankHit()
{
    bulletNode_t *tHead = alienBulletList;
    while (tHead != NULL)
    {
        double dist = getDistance(tHead->bullet.posX, tHead->bullet.posY + BULLET_HEIGHT, tank.posX + (tank.width / 2), tank.posY - (tank.height / 2));
        if (dist <= 5.5)
        {
            tHead->bullet.alive = 0;
            lives -= 1;
            resetTankPosition();
            if (lives == 0) {
                GAME_STATE = GAME_STATE_GAME_OVER;
            }
        }
        tHead = tHead->next;
    }
}

void processInput()
{
    int rightVal = digitalRead(RIGHT);
    int leftVal = digitalRead(LEFT);
    int fireVal = digitalRead(FIRE);
    // Serial.printf("%d  %d   %d\n", leftVal, rightVal, fireVal);
    if (GAME_STATE == GAME_STATE_RUN) {
        if (rightVal == 0 && (tank.posX + tank.width < SCREEN_WIDTH))
        {
            tank.posX += tank.speed;
        }
        if (leftVal == 0 && tank.posX > 0)
        {
            tank.posX -= tank.speed;
        }
        if (fireVal == 0)
        {
            if (fired == 0)
            {
                spawnTankBullet(tank.posX + (tank.width / 2) - 1, tank.posY - BULLET_HEIGHT);
                fired = 1;
            }
        }
        else
        {
            fired = 0;
        }
    } else if (GAME_STATE == GAME_STATE_MENU) {
        if (fireVal == 0) {
            GAME_STATE = GAME_STATE_RUN;
        }
    } else if (GAME_STATE == GAME_STATE_GAME_OVER) {
        if (fireVal == 0) {
            starGame();
            GAME_STATE = GAME_STATE_RUN;
        }
    }
}

void space_invaders_update()
{
    processInput();

    if (GAME_STATE == GAME_STATE_RUN) {
        updateAlienPositions();
        updateTankBullets();
        spawnRandomBullet();
        updateAlienBullets();
        checkAlienHit();
        checkTankHit();
        cleanTankBullets();
        cleanAlienBullets();
    }
}

double getDistance(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    return (double)sqrt(pow((x2 - x1), 2.0) + pow((y2 - y1), 2.0));
}

void space_invaders_setup()
{
    alienBulletTimeRemaining = ALIEN_BULLET_SPAWN_TIME;
    starGame();
}
