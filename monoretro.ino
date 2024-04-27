#include "pong.h"
#include "space_invaders.h"

void setup() {
    pongSetup();
}

void loop()
{
    pongUpdate();
    pongRender();
    delay((int)(1000 / 60));
}


