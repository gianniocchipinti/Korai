#include "stm32f4xx_hal.h"  // HAL libraries for STM32
#include "u8g2.h"           // u8g2 library for OLED display
#include "FreeRTOS.h"      // FreeRTOS header
#include "task.h"          // FreeRTOS task functions
#include "ff.h"            // FatFs header for file access
// CC1101 GDO0 pin connected to STM32 GPIOA pin 0
#define CC1101_GDO0_PORT GPIOA
#define CC1101_GDO0_PIN  GPIO_PIN_0

// CC1101 CSn pin connected to STM32 GPIOC pin 4
#define CC1101_CSn_PORT  GPIOC
#define CC1101_CSn_PIN   GPIO_PIN_4

// CC1101 SI pin connected to STM32 GPIOC pin 5
#define CC1101_SI_PORT   GPIOC
#define CC1101_SI_PIN    GPIO_PIN_5

// CC1101 SO pin connected to STM32 GPIOC pin 6
#define CC1101_SO_PORT   GPIOC
#define CC1101_SO_PIN    GPIO_PIN_6

// CC1101 SCLK pin connected to STM32 GPIOC pin 7
#define CC1101_SCLK_PORT GPIOC
#define CC1101_SCLK_PIN  GPIO_PIN_7

const uint32_t frequency_list[] = {
    868000000,  // 868 MHz
    900000000,  // 900 MHz
    915000000,  // 915 MHz
    928000000   // 928 MHz
};

const uint8_t num_frequencies = sizeof(frequency_list) / sizeof(frequency_list[0]);

void init() {
    // Initialize CC1101
    cc1101_init();

    // Initialize OLED display
    u8g2_InitDisplay(&u8g2);  // send init sequence to the display
    u8g2_SetPowerSave(&u8g2, 0); // wake up display
}

void change_frequency(uint32_t frequency) {
    cc1101_set_frequency(frequency);
}
void select_frequency() {
    // Display frequency selection menu on OLED display
    u8g2_ClearBuffer(&u8g2);
    for (int i = 0; i < num_frequencies;
void set_frequency(uint8_t index) {
    // Assert the CSn pin (active low)
    HAL_GPIO_WritePin(CC1101_CSn_PORT, CC1101_CSn_PIN, GPIO_PIN_RESET);

    // Set the SI pin to the appropriate value (1 for frequency_list[index], 0 for all other frequencies)
    if (index < num_frequencies) {
        HAL_GPIO_WritePin(CC1101_SI_PORT, CC1101_SI_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(CC1101_SI_PORT, CC1101_SI_PIN, GPIO_PIN_RESET);
    }

    // Toggle the SCLK pin to latch the data
    HAL_GPIO_TogglePin(CC1101_SCLK_PORT, CC1101_SCLK_PIN);

    // Deassert the CSn pin
    HAL_GPIO_WritePin(CC1101_CSn_PORT, CC1101_CSn_PIN, GPIO_PIN_SET);
}
void select_frequency() {
    // Display frequency selection menu on OLED display
    u8g2_ClearBuffer(&u8g2);
    for (int i = 0; i < num_frequencies; i++) {
        u8g2_DrawStr(&u8g2, 0, i*10, std::to_string(frequency_list[i]).c_str());
    }
    u8g2_SendBuffer(&u8g2);

    // Wait for user to select a frequency
    uint8_t index = 0;
    while (1) {
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_SET) {
            // User has selected a frequency
            set_frequency(index);
            break;
        } else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_SET) {
            // User has requested to move to the next frequency
            index++;
            if (index >= num_frequencies) {
                index = 0;
            }
            u8g2_ClearBuffer(&u8g2);
            for (int i = 0; i < num_frequencies; i++) {
                u8g2_DrawStr(&u8g2, 0, i*10, std::to_string(frequency_list[i]).c_str());
            }
void radio_task(void *pvParameters) {
    // Define variables for reading from file
    FIL file;
    UINT bytes_read;
    uint8_t buffer[1024];  // buffer for reading data from file

    // Open the file
    f_open(&file, "/data/message.txt", FA_READ);

    // Read data from the file and send it over the radio
    while (f_read(&file, buffer, sizeof(buffer), &bytes_read) == FR_OK && bytes_read > 0) {
        // Send the data over the radio
        send_data_over_radio(buffer, bytes_read);
    }

    // Close the file
    f_close(&file);

    // Delete the task
    vTaskDelete(NULL);
}
xTaskCreate(radio_task, "Radio Task", 1024, NULL, 1, NULL);
void send_data_over_radio(uint8_t *data, uint16_t length) {
    // Assert the CSn pin (active low)
    HAL_GPIO_WritePin(CC1101_CSn_PORT, CC1101_CSn_PIN, GPIO_PIN_RESET);

    // Send the "write data" command to the CC1101
    cc1101_write_command(CC1101_CMD_WRITE_TX_FIFO);

    // Send the data to the CC1101 over the SPI interface
    cc1101_write_data(data, length);

    // Deassert the CSn pin
    HAL_GPIO_WritePin(CC1101_CSn_PORT, CC1101_CSn_PIN, GPIO_PIN_SET);

    // Send the "transmit" command to the CC1101
    cc1101_write_command(CC1101_CMD_TX);
}
void cc1101_write_command(uint8_t command) {
    // Assert the CSn pin (active low)
    HAL_GPIO_WritePin(CC1101_CSn_PORT, CC1101_CSn_PIN, GPIO_PIN_RESET);

    // Set the SI pin to 0 (command mode)
    HAL_GPIO_WritePin(CC1101_SI_PORT, CC1101_SI_PIN, GPIO_PIN_RESET);

    // Toggle the SCLK pin to latch the data
    HAL_GPIO_TogglePin(CC1101_SCLK_PORT, CC1101_SCLK_PIN);

    // Send the command over the SPI interface
    for (int i = 7; i >= 0; i--) {
        // Set the SI pin to the appropriate value
        HAL_GPIO_WritePin(CC1101_SI_PORT, CC1101_SI_PIN, (command & (1 << i)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

        // Toggle the SCLK pin to latch the data
        HAL_GPIO_TogglePin(CC1101_SCLK_PORT, CC1101_SCLK_PIN);
    }

    // Deassert the CSn pin
    HAL_GPIO_WritePin(CC1101_CSn_PORT, CC1101_CSn_PIN, GPIO_PIN_SET);
}
void cc1101_write_data(uint8_t *data, uint16_t length) {
    // Assert the CSn pin (active low)
    HAL_GPIO_WritePin(CC1101_CSn_PORT, CC1101_CSn_PIN, GPIO_PIN_RESET);

    // Set the SI pin to 1 (data mode)
    HAL_GPIO_WritePin(CC1101_SI_PORT, CC1101_SI_PIN, GPIO_PIN_SET);

    // Toggle the SCLK pin to latch the data
    HAL_GPIO_TogglePin(CC1101_SCLK_PORT, CC1101_SCLK_PIN);

    // Send the data over the SPI interface
    for (int i = 0; i < length; i++) {
        for (int j = 7; j >= 0; j--) {
            // Set the SI pin to the appropriate value
            HAL_GPIO_WritePin(CC1101_SI_PORT, CC1101_SI_PIN, (data[i] & (1 << j)) ? GPIO_PIN_SET : GPIO_PIN_RESET);

            // Toggle the SCLK pin to latch the data
            HAL_GPIO_TogglePin(CC1101_SCLK_PORT, CC1101_SCLK_PIN);
        }
    }

    // Deassert the CSn pin
    HAL_GPIO_WritePin(CC1101_CSn_PORT, CC1101_CSn_PIN, GPIO_PIN_SET);
}
