#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Button.hpp"
#include "File.hpp"
#include "Book.hpp"
#include "Display.hpp"
#include "SDManager.hpp"
#include "states/BookState.hpp"
#include "Text/Rendering/TextBox.hpp"
#include "Text/Support/TextHelper.hpp"
#include <Fonts/hebEng5x7avia.h>

#include <string_view>
#include <esp_log.h>
#include <string>
#include <exception>
#include <memory>

static constexpr std::string_view MAIN_TAG = "Main";
static constexpr std::string_view BOOK_NAME = "percy_2_heb";
static constexpr gpio_num_t LIGHT_PIN = GPIO_NUM_22;
static constexpr gpio_num_t RIGHT = GPIO_NUM_25;
static constexpr gpio_num_t LEFT = GPIO_NUM_26;
static constexpr gpio_num_t MIDDLE = GPIO_NUM_27;
static bool LIGHT_STATE = true;

static void toggle_light() {
    gpio_set_level(LIGHT_PIN, LIGHT_STATE);
    LIGHT_STATE = !LIGHT_STATE;
}

static void start_light() {
    if (!GPIO_IS_VALID_OUTPUT_GPIO(LIGHT_PIN)) {
        ESP_LOGE(MAIN_TAG.data(), "GPIO%d cannot drive the light; use an output-capable pin", LIGHT_PIN);
        return;
    }
    ESP_ERROR_CHECK(gpio_set_level(LIGHT_PIN, 0));
    ESP_ERROR_CHECK(gpio_set_direction(LIGHT_PIN, GPIO_MODE_OUTPUT));
    toggle_light();
}

extern "C" void app_main(void)
{
    // Adafruit_GFX_Button ui_button1{};
    // ui_button1.initButton(g_display.get(), 140, 50,  100, 60, 0, 1, 0, (char*)"Button", 1);
    // ui_button1.drawButton();
    try
    {
        start_light();
        ESP_LOGI(MAIN_TAG.data(), "Mount SD card");
        SDManager::init();
        ESP_LOGI(MAIN_TAG.data(), "Load book and initialize display");
        std::unique_ptr<ProgramState> current_state = nullptr;
        current_state = std::make_unique<BookState>(BOOK_NAME, std::make_unique<Display>(SPI::create_display_spi(), Color::WHITE));

        ESP_LOGI(MAIN_TAG.data(), "Render initial book state");
        current_state->main();
        
        Button right_button(RIGHT);
        Button left_button(LEFT);
        Button middle_button(MIDDLE);
        ESP_LOGI(MAIN_TAG.data(), "Initialize buttons");
        right_button.init();
        left_button.init();
        middle_button.init();

        static const char* TAG = "Button";

        while (true) {
            right_button.update();
            left_button.update();
            middle_button.update();

            const Button::Event middle_event = middle_button.get_state();
            if (middle_event == Button::Event::Click) {
                    ESP_LOGI(TAG, "Middle click");
                    current_state->on_hold();
            }

            else if (middle_event == Button::Event::LongPress) {
                ESP_LOGI(TAG, "Middle hold press");
                toggle_light();
            }

            else if (right_button.get_state() == Button::Event::Click) {
                    ESP_LOGI(TAG, "Right click");
                    current_state->on_click();
            }

            else if (left_button.get_state() == Button::Event::Click) {
                    ESP_LOGI(TAG, "Left click");
                    current_state->on_double_click();
            }

            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
    catch (const std::exception& e)
    {
        ESP_LOGE(MAIN_TAG.data(), "Exception: %s", e.what());
    }
    catch (...)
    {
        ESP_LOGE(MAIN_TAG.data(), "Unknown exception");
    }
}
