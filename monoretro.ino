#include "monoretro.h"
#include "pong.h"
#include "space_invaders.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// menu selecter 9
// down 10
// up 11

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 monoDisplay(SCREEN_WIDTH, SCREEN_HEIGHT);

const char* options[] = {"PONG", "SPACE INVADERS"};
uint8_t selectedOption = 0;
uint8_t MACHINE_STATE = MACHINE_STATE_MENU;


void setup() {
    Serial.begin(115200);
    if (!monoDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;)
            ; // Don't proceed, loop forever
    }

    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    monoDisplay.display();
    // Clear the buffer
    monoDisplay.clearDisplay();
    pinMode(SELECTOR, INPUT_PULLUP);
    pinMode(UP, INPUT_PULLUP);
    pinMode(DOWN, INPUT_PULLUP);

    //pongSetup();
}

void gameSetup() {
    if (selectedOption == 0) {
        pongSetup();
    } else if (selectedOption == 1) {
        space_invaders_setup();
    }
}

static void monoProcessInput()
{
    int upVal = digitalRead(UP);
    int downVal = digitalRead(DOWN);
    int selector = digitalRead(SELECTOR);
    
    if (MACHINE_STATE == MACHINE_STATE_MENU) {
        if (upVal == 0 && selectedOption > 0) { 
            selectedOption -= 1;    
        }   
        if (downVal == 0 && selectedOption < 1) {   
            selectedOption += 1;    
        }   
    }
    if (selector == 0) {
        if (MACHINE_STATE == MACHINE_STATE_MENU) {
            MACHINE_STATE = MACHINE_STATE_GAME;
            gameSetup();
        } else if (MACHINE_STATE == MACHINE_STATE_GAME) {
            MACHINE_STATE = MACHINE_STATE_MENU;
            selectedOption = 0;
        }
    }
}

void renderMenu() {
    uint8_t xOffset = 24;
    uint8_t yOffset = 10;
    uint8_t yOffsetDiff = 20;
    monoDisplay.clearDisplay();
    monoDisplay.setTextSize(1);
    monoDisplay.setTextColor(WHITE);
    for (size_t i = 0; i < 2; i++) {
        monoDisplay.setCursor(xOffset, yOffset);
        if (i == selectedOption) {
            monoDisplay.printf("> %s", options[i]);
        } else {
            monoDisplay.printf("%s", options[i]);
        }
        yOffset += yOffsetDiff;
    }
    monoDisplay.display(); 
}

void monoUpdate() {
    if (MACHINE_STATE == MACHINE_STATE_GAME) {
        if (selectedOption == 0) {
            pongUpdate();
        } else if (selectedOption == 1) {
            space_invaders_update();
        }
    }
}

void monoRender() {
    if (MACHINE_STATE == MACHINE_STATE_MENU) {
        renderMenu();
    } else if (MACHINE_STATE == MACHINE_STATE_GAME) {
        if (selectedOption == 0) {
            pongRender();
        } else if (selectedOption == 1) {
            space_invaders_render();
        }
    }
}

void loop()
{
    //pongUpdate();
    //pongRender();
    monoProcessInput();
    monoUpdate();
    monoRender();
    delay((int)(1000 / 60));
}


