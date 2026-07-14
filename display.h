#pragma once

#include <vector>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define OLED_RESET -1

class Display
{
private:
    Adafruit_SSD1306 display;

public:
    Display()
        : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET)
    {
    }

    void Begin()
    {
        Wire.begin();

        if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
        {
            Serial.println("OLED init failed");
            return;
        }

        display.clearDisplay();
        display.display();
    }

    void PrintString(const String &str)
    {
        display.clearDisplay();

        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(0, 0);

        display.println(str);

        display.display();
    }

    void PrintImage(const std::vector<bool> &image)
    {
        if (image.size() != SCREEN_WIDTH * SCREEN_HEIGHT)
            return;

        display.clearDisplay();

        for (int y = 0; y < SCREEN_HEIGHT; y++)
        {
            for (int x = 0; x < SCREEN_WIDTH; x++)
            {
                if (image[y * SCREEN_WIDTH + x])
                    display.drawPixel(x, y, SSD1306_WHITE);
            }
        }

        display.display();
    }

    void Clear()
    {
        display.clearDisplay();
        display.display();
    }
};