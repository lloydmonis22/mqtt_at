/*
 * app.c
 *
 *  Created on: Feb 5, 2025
 *      Author: shrey
 */

#include "app.h"
#include "esp8266.h"
#include <string.h>
#include "main.h"
#include <stdio.h>

#define MAX_PUB_MSG_SIZE     128
#define MAX_INCOMING_BUFFER  MAX_BUFFER_SIZE


//-----------------------------------------------------------------------------
// This function publishes a message and then waits for an incoming response.
// It then checks the received message for LED control commands ("LED ON" or "LED OFF")
// and controls the LED accordingly.
//-----------------------------------------------------------------------------
int32_t publish_and_process_incoming_message(void)
{
    static uint32_t counter = 0;
    char pubMessage[MAX_PUB_MSG_SIZE];
    uint8_t messageBuffer[MAX_INCOMING_BUFFER];
    const uint8_t *token = (const uint8_t *)"OK";

    sprintf(pubMessage, "hello aws! Count: %lu", counter++);

    // Publish the message to "topic/esp32at" with QoS 1 and no retain
    if (esp8266_mqtt_publish("topic/esp32at", pubMessage, 1, 0) != ESP8266_OK)
    {
        Error_Handler();
    }
#if 0
    // Optional delay to give the module time to send its response
    HAL_Delay(100);

    // Process incoming message until we see the token (e.g., "OK")
    if (catch_incoming_message(messageBuffer, MAX_INCOMING_BUFFER, token) != ESP8266_OK)
    {
        // Optionally print the received message for debugging
        printf("Received message: %s\n", messageBuffer);

        // Check if the message contains LED control commands.
        if (strstr((char *)messageBuffer, "LED ON") != NULL)
        {
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
            printf("LED turned ON\n");
        }
        else if (strstr((char *)messageBuffer, "LED OFF") != NULL)
        {
            HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
            printf("LED turned OFF\n");
        }
        return 0;  // Success
    }
    else
    {
        // Handle error in receiving expected message
        printf("Error: Token '%s' not found in incoming message\n", token);
        return -1;
    }
#endif
}
