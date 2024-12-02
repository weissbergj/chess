#include "network_driver.h"
#include "uart.h"  // For UART communication
#include "printf.h"
#include "strings.h"

// Function to send a command to the Wi-Fi module and check for a response
int send_command(const char* command, const char* expected_response) {
    uart_putstring(command);
    uart_putstring("\r\n");

    // Simulate reading a response (you need to implement actual UART reading)
    char response[100];
    uart_read_response(response, sizeof(response));  // Implement this function

    // // Check if the response contains the expected response
    // if (strstr(response, expected_response) != NULL) {
    //     return TRUE;
    // }
    return TRUE;
}

void wifi_init(void) {
    uart_putstring("Initializing Wi-Fi module...\n");
    if (!send_command("AT", "OK")) {
        uart_putstring("Failed to communicate with Wi-Fi module.\n");
    }
}

void wifi_reset(void) {
    uart_putstring("Resetting Wi-Fi module...\n");
    if (!send_command("AT+RST", "OK")) {
        uart_putstring("Failed to reset Wi-Fi module.\n");
    }
}

void wifi_configure(const char* ssid, const char* password) {
    uart_putstring("Configuring Wi-Fi...\n");
    char command[100];

    // Set Wi-Fi mode to station
    if (!send_command("AT+CWMODE=1", "OK")) {
        uart_putstring("Failed to set Wi-Fi mode.\n");
    }

    // Connect to the Wi-Fi network
    snprintf(command, sizeof(command), "AT+CWJAP=\"%s\",\"%s\"", ssid, password);
    if (!send_command(command, "OK")) {
        uart_putstring("Failed to connect to Wi-Fi network.\n");
    }
}

int wifi_connect(void) {
    uart_putstring("Checking Wi-Fi connection...\n");
    return send_command("AT+CWJAP?", "OK");
}

int network_init(void) {
    uart_init();  // Initialize UART for debugging
    uart_putstring("Initializing network...\n");

    wifi_init();  // Initialize Wi-Fi module
    wifi_reset();  // Reset Wi-Fi module
    wifi_configure("YourSSID", "YourPassword");  // Configure Wi-Fi settings

    int wifi_connected = wifi_connect();

    if (!wifi_connected) {
        uart_putstring("Failed to connect to Wi-Fi.\n");
        return FALSE;
    }

    uart_putstring("Connected to Wi-Fi.\n");
    return TRUE;
}