#pragma once

#include <Arduino.h>

#include <Adafruit_ThinkInk.h>

#include "prediction.hpp"

#define COLOR1 EPD_BLACK
#define COLOR2 EPD_LIGHT
#define COLOR3 EPD_DARK


class DisplayHandle
{
public:
    DisplayHandle(ThinkInk_290_Grayscale4_T5 *display_pointer, tm* time_info_ptr) : display(display_pointer), time_info(time_info_ptr) {}

    void create_battery_string(char *fill_batt, float battery_pct)
    {
        sprintf(fill_batt, "Batt Voltage: %2.0f %%", battery_pct);
        Serial.println();
    }

void PrepDisplay()
{
    display->begin(THINKINK_MONO);
    display->clearBuffer();
    display->setTextSize(1);
    display->setCursor(0, 1);
    display->setTextColor(COLOR1);
    display->setTextWrap(true);
}

void WriteBanner(float battery_voltage)
{
    char batt_str[30];
    create_battery_string(batt_str, battery_voltage);
    display->print(batt_str);
    display->setCursor(150, 1);
    display->print(time_info, "%B %d %Y %H:%M");
    display->drawFastHLine(0, 14, display->width(), COLOR1);
}

void write_predictions(StopPrediction predictions[], int num_predictions)
{
    // Write an array of predictions to the screen
    int line_number = 0;
    for (int s = 0; s <= num_predictions; s++)
    {
        auto pred = predictions[s];
        Serial.print("Writing prediction to screen");
        int cursor_pos_y = 16 * (line_number + 1);
        display->setCursor(1, cursor_pos_y);
        //Serial.printf("set curser to y: %d \n", cursor_pos_y);
        //Serial.println("write string to screen buff");
        int arriving_in = pred.get_countdown_min(*time_info);
        //Serial.printf("On print object %d \n", s);
        if (arriving_in < 0)
        {
            continue;
        }
        if (line_number > 6)
        {
            break;
        }
        if (strcmp(pred.via(), "0") == 0)
        {
            display->printf("Bus: %s %d Min", pred.route(), arriving_in);
        }
        else
        {
            display->printf("Bus: %s %d Min v %s", pred.route(), arriving_in, pred.via());
        }
        line_number++;
    }
}

void display_data()
{
    display->display();
}

private:
    ThinkInk_290_Grayscale4_T5 *display;
    tm* time_info;
};