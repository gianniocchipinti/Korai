#include <stdio.h>
#include <string.h>
#include "cc1101.h"
#include "stm32wbxx.h"
#include "u8g2.h"
#include "fatfs.h"
#include "FreeRTOS.h"

// Global variables for storing the selected frequency, file name, and recording status
int frequency;
char file_name[256];
bool recording;

// Function prototypes
void select_frequency();
void send_file();
void read_signals();
void save_signals();

int main()
{
    // Initialize the CC1101 chip and STM32WB55
    cc1101_init();
    stm32wb55_init();

    // Initialize the U8G2 library and display the main menu
    u8g2_t u8g2;
    u8g2_init(&u8g2, ...); // Initialize the display
    while (1)
    {
        select_frequency();
        u8g2_clear_buffer(&u8g2);
        u8g2_draw_string(&u8g2, 0, 0, "Frequency: %d", frequency);
        u8g2_draw_string(&u8g2, 0, 10, "1. Send");
        u8g2_draw_string(&u8g2, 0, 20, "2. Read");
        u8g2_send_buffer(&u8g2);

        // Wait for the user to make a selection
        int choice = get_user_input();
        if (choice == 1)
        {
            send_file();
        }
        else if (choice == 2)
        {
            read_signals();
        }
    }

    return 0;
}

void select_frequency()
{
    // Display a menu with a list of frequencies and allow the user to select one
    u8g2_clear_buffer(&u8g2);
    u8g2_draw_string(&u8g2, 0, 0, "Select a frequency:");
    u8g2_draw_string(&u8g2, 0, 10, "1. 433 MHz");
    u8g2_draw_string(&u8g2, 0, 20, "2. 868 MHz");
    // Add more options for other frequencies as needed
    u8g2_send_buffer(&u8g2);
    int choice = get_user_input();
    if (choice == 1)
    {
        frequency = 433;
    }
    else if (choice == 2)
    {
        frequency = 868;
    }
    else
    {
        // Handle invalid input
    }
    // Set the frequency on the CC1101 chip
    cc1101_set_frequency(frequency);
}

void send_file()
{
    // Use the FatFs and FreeRTOS libraries to allow the user to select a file from the internal storage
    FRESULT res;
    DIR dir;
    static FILINFO fno;

    // Select a file using the FatFs and FreeRTOS libraries
    strcpy(file_name, ""); // Clear the file name
    res = select_file_from_storage(&dir, &fno, file_name);
    if (res != FR_OK)
    {
        // An error occurred while trying to select a file
        return;
    }

    // Open the file and read its contents into a buffer
    FIL file;
    res = f_open(&file, file_name, FA_READ);
    if (res != FR_OK)
    {
        // An error occurred while trying to open the file
        return;
    }
    char buffer[1024]; // Use a buffer to hold the file contents
    UINT bytes_read;
    f_read(&file, buffer, sizeof(buffer), &bytes_read);
    f_close(&file);

    // Transmit the contents of the file using the CC1101 chip
    cc1101_transmit(buffer, bytes_read);

    // Display a message to the user indicating that the transmission is complete
    u8g2_clear_buffer(&u8g2);
    u8g2_draw_string(&u8g2, 0, 0, "Transmission complete.");
    u8g2_send_buffer(&u8g2);
    delay(1000); // Wait for 1 second before returning to the main menu
}

void read_signals()
{
    // Set the recording flag to true
    recording = true;

    // Display a message to the user indicating that the recording has started
    u8g2_clear_buffer(&u8g2);
    u8g2_draw_string(&u8g2, 0, 0, "Recording started.");
    u8g2_send_buffer(&u8g2);

    // Initialize a buffer to hold the recorded signals
    char buffer[1024];
    int bytes_read = 0;

    // Keep reading signals until the recording flag is set to false
    while (recording)
    {
        // Read a signal using the CC1101 chip
        int signal = cc1101_receive();
        if (signal != -1)
        {
            // Add the signal to the buffer
            buffer[bytes_read] = (char)signal;
            bytes_read++;
        }
    }

    // Save the recorded signals to a file
    save_signals(buffer, bytes_read);

    // Display a message to the user indicating that the recording is complete
    u8g2_clear_buffer(&u8g2);
    u8g2_draw_string(&u8g2, 0, 0, "Recording complete.");
    u8g2_send_buffer(&u8g2);
    delay(1000); // Wait for 1 second before returning to the main menu
}

void save_signals(char *buffer, int bytes_read)
{
    // Use the FatFs and FreeRTOS libraries to save the recorded signals to a file
    FRESULT res;
    DIR dir;
    static FILINFO fno;

    // Select a file using the FatFs and FreeRTOS libraries
    strcpy(file_name, ""); // Clear the file name
    res = select_file_from_storage(&dir, &fno, file_name);
    if (res != FR_OK)
    {
        // An error occurred while trying to select a file
        return;
    }

    // Open the file and write the recorded signals to it
    FIL file;
    res = f_open(&file, file_name, FA_WRITE | FA_CREATE_ALWAYS);
    if (res != FR_OK)
    {
        // An error occurred while trying to open the file
        return;
    }
    UINT bytes_written;
    f_write(&file, buffer, bytes_read, &bytes_written);
    f_close(&file);
}
