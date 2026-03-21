#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "epd042.h"
#include "Fonts/FreeMonoBold9pt7b.h"
#include "Fonts/heb5x7A.h"
#include "Fonts/hebEng5x7avia.h"

static const uint16_t EPD_BLACK = 0x0000;
static const uint16_t EPD_WHITE = 0xFFFF;

Epd042 display;
static const int16_t HEBREW_START_X = 390;
static const int16_t HEBREW_START_Y = 1;
static const int16_t HEBREW_CHAR_Y_SIZE = 9;

extern "C" void app_main(void)
{
    printf("=== Hebrew Font Test ===\n");

    display.init();
    display.fillScreen(EPD_WHITE);
    display.setRotation(2);
    display.setTextColor(EPD_BLACK);
    display.setFont(&hebEng5x7avia);

    display.setTextSize(2);
    display.setCursor(HEBREW_START_X, HEBREW_START_Y);
    display.printHebrew("אבגדהוזחטיכלמנסעפצקרשת");
    display.printHebrew("ךםןףץ");
    //display.printHebrew("הואעולהכלבוקרעלאוטובוסיםעצובים");
    display.setCursor(390, HEBREW_START_Y + HEBREW_CHAR_Y_SIZE * 2);
    //display.printHebrew("אפילו הם יודעים לבכות היום");
    display.printHebrew("אני רוצה to commit סואסייד");
    // display.setCursor(0, 40);
    // display.println("to commit");
    printf("Updating display...\n");
    display.update();
    printf("Done!\n");
    display.deepsleep();
    // while (1) {
    //     vTaskDelay(pdMS_TO_TICKS(1000));
    // }
}
