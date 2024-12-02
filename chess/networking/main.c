#include "network_driver.h"
// #include "tcp_ip_stack.h"
// #include "http_client.h"
#include "printf.h"  // Your custom printf header

void setup_network() {
    // Initialize the network driver
    if (!network_init()) {
        printf("Network initialization failed!\n");
        return;
    }
    printf("Network initialized successfully.\n");
}

// void perform_http_request() {
//     // Create an HTTP client
//     HttpClient client;
//     http_client_init(&client);

//     // Set the server details
//     const char *server = "example.com";
//     const char *path = "/path";

//     // Perform the HTTP GET request
//     if (http_client_get(&client, server, path)) {
//         printf("HTTP GET request successful.\n");
//         printf("Response: %s\n", client.response);
//     } else {
//         printf("HTTP GET request failed.\n");
//     }

//     // Clean up
//     http_client_cleanup(&client);
// }

int main() {
    // Initialize any necessary hardware, such as UART, if required by printf
    setup_network();
    // perform_http_request();

    while (1) {
        // Main loop - could handle other tasks or sleep
    }

    return 0;
}