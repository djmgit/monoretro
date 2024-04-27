#ifndef SPACE_INVADERS_H
#define SPACE_INVADERS_H

void space_invaders_setup();

void space_invaders_update();

void space_invaders_render();

typedef struct
{
    uint8_t posX;
    uint8_t posY;
    uint8_t width;
    uint8_t height;
    uint8_t alienType;
    uint8_t alive;
} alien_t;

typedef struct
{
    uint8_t posX;
    uint8_t posY;
    uint8_t width;
    uint8_t height;
    uint8_t alive;
    uint8_t speed;
} player_t;

typedef struct
{
    uint8_t posX;
    uint8_t posY;
    uint8_t speed;
    uint8_t alive;
} bullet_t;

void loadAliens();
void spawnTankBullet(uint8_t, uint8_t);
double getDistance(uint8_t, uint8_t, uint8_t, uint8_t);
void spawnAlienBullet(uint8_t, uint8_t);
alien_t spawnAlien(uint8_t, uint8_t, uint8_t);

#endif
