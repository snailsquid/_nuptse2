#include "display.h"

Arduino_GFX *gfx;

static Arduino_DataBus *bus;

#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define DARKGREY 0x528A

void initDisplay()
{
    bus = new Arduino_ESP32SPI(17 /* DC */, GFX_NOT_DEFINED /* no CS */, 18 /* SCK */, 23 /* MOSI */, GFX_NOT_DEFINED);
    gfx = new Arduino_ST7789(bus, 16 /* RST */, 3 /* rotation */, true /* IPS */, 240, 240, 0, 80, 0, 80);

    if (!gfx->begin()) {
        Serial.println("Screen failed to begin!");
        while (1);
    }

    gfx->fillScreen(BLACK);
    gfx->setTextWrap(false);
}

void drawIdleScreen()
{
    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);

    gfx->setCursor(20, 40);
    gfx->setTextSize(3);
    gfx->println("Pulse");
    gfx->setCursor(20, 75);
    gfx->println("Oximeter");

    gfx->setTextSize(1);
    gfx->setTextColor(RED);
    gfx->setCursor(50, 140);
    gfx->println("No Finger Detected");

    gfx->setTextColor(DARKGREY);
    gfx->setCursor(30, 180);
    gfx->println("Place finger on sensor");
}

void drawCalibrateScreen(int calCount)
{
    static int prevCount = -1;
    if (calCount == prevCount) return;
    prevCount = calCount;

    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(20, 50);
    gfx->println("Calibrating...");

    int barWidth = 180;
    int barHeight = 16;
    int barX = 30;
    int barY = 100;
    int progress = (calCount * barWidth) / 150;
    if (progress > barWidth) progress = barWidth;

    gfx->drawRect(barX, barY, barWidth, barHeight, WHITE);
    if (progress > 0)
        gfx->fillRect(barX + 1, barY + 1, progress - 2, barHeight - 2, YELLOW);

    gfx->setTextSize(1);
    gfx->setCursor(60, 140);
    gfx->setTextColor(YELLOW);
    gfx->print("Samples: ");
    gfx->println(calCount);
}

void drawReadyScreen()
{
    gfx->fillScreen(BLACK);
    gfx->setTextColor(BLUE);
    gfx->setTextSize(3);
    gfx->setCursor(60, 60);
    gfx->println("Ready");

    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);
    gfx->setCursor(30, 120);
    gfx->println("Press button to start");
    gfx->setCursor(30, 145);
    gfx->println("measurement");
}

void drawCalculateScreen(int bpm, unsigned long elapsed)
{
    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(20, 30);
    gfx->print("Time: ");
    int timeLeft = 60 - (int)(elapsed);
    gfx->print(timeLeft);
    gfx->println("s");

    gfx->setTextSize(4);
    gfx->setTextColor(GREEN);
    gfx->setCursor(30, 90);
    gfx->print(bpm);
    gfx->setTextSize(2);
    gfx->setCursor(30, 135);
    gfx->println("BPM");
}

void drawFinishedScreen(double delta)
{
    gfx->fillScreen(BLACK);

    gfx->setTextSize(2);
    gfx->setTextColor(GREEN);
    gfx->setCursor(40, 30);
    gfx->println("Complete!");

    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);
    gfx->setCursor(20, 80);
    gfx->print("Delta: ");
    gfx->setTextColor(YELLOW);
    gfx->print(delta, 1);
    gfx->println(" BPM");

    gfx->setTextColor(DARKGREY);
    gfx->setCursor(20, 140);
    gfx->println("Remove finger to reset");
}

void drawFailScreen()
{
    gfx->fillScreen(BLACK);

    gfx->setTextSize(3);
    gfx->setTextColor(RED);
    gfx->setCursor(50, 80);
    gfx->println("FAIL");

    gfx->setTextSize(1);
    gfx->setTextColor(WHITE);
    gfx->setCursor(20, 140);
    gfx->println("Remove finger and retry");
}

void updateDisplay(State state, int bpm, double delta, unsigned long calcStart, int calCount)
{
    static State lastState = (State)-1;
    static unsigned long lastCalcDraw = 0;

    switch (state) {
        case IDLE:
            if (lastState != IDLE) { drawIdleScreen(); lastState = IDLE; }
            break;
        case CALIBRATE:
            drawCalibrateScreen(calCount);
            break;
        case READY:
            if (lastState != READY) { drawReadyScreen(); lastState = READY; }
            break;
        case CALCULATE:
            {
                unsigned long now = millis();
                if (now - lastCalcDraw < 100) break;
                lastCalcDraw = now;
                unsigned long elapsed = (millis() - calcStart) / 1000;
                drawCalculateScreen(bpm, elapsed);
            }
            break;
        case FINISHED:
            if (lastState != FINISHED) { drawFinishedScreen(delta); lastState = FINISHED; }
            break;
        case FAIL_STATE:
            if (lastState != FAIL_STATE) { drawFailScreen(); lastState = FAIL_STATE; }
            break;
    }
}
