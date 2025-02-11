/*
 * esp8266_io.c
 *
 *  Created on: Dec 27, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

/* Includes ------------------------------------------------------------------*/
#include "esp8266_io.h"
#include "main.h"
#include <string.h>

/* Private define ------------------------------------------------------------*/
#define RING_BUFFER_SIZE        (1024 * 8)

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t  data[RING_BUFFER_SIZE];
  uint16_t tail;
  uint16_t head;
} ring_buffer_t;

/* Private variables ---------------------------------------------------------*/
ring_buffer_t wifi_rx_buffer;
UART_HandleTypeDef *wifi_uart_handle;
DMA_HandleTypeDef *wifi_dma_handle;

/* Private function prototypes -----------------------------------------------*/
static void esp8266_io_error_handler(void);

/* Exported functions -------------------------------------------------------*/

/**
  * @brief  Initialize the ESP8266 UART interface with DMA and Idle Detection.
  * @retval 0 on success, -1 otherwise.
  */
int8_t esp8266_io_init(void)
{
  wifi_rx_buffer.head = 0;
  wifi_rx_buffer.tail = 0;

  // Start UART in DMA mode with Idle line detection
  if (HAL_UARTEx_ReceiveToIdle_DMA(wifi_uart_handle, wifi_rx_buffer.data, RING_BUFFER_SIZE) != HAL_OK)
  {
      return -1;
  }

  return 0;
}

/**
  * @brief  Deinitialize the ESP8266 UART interface.
  * @retval None.
  */
void esp8266_io_deinit(void)
{
    HAL_UART_DMAStop(wifi_uart_handle);
    HAL_UART_DeInit(wifi_uart_handle);
}

/**
  * @brief  Send data to the ESP8266 module over UART.
  * @param  p_data: Pointer to the data buffer to send.
  * @param  length: Length of the data buffer.
  * @retval 0 on success, -1 otherwise.
  */
int8_t esp8266_io_send(uint8_t* p_data, uint32_t length)
{
  if (HAL_UART_Transmit(wifi_uart_handle, p_data, length, DEFAULT_TIME_OUT) != HAL_OK)
  {
      return -1;
  }
  return 0;
}

/**
  * @brief  Receive data from the ESP8266 module over UART.
  * @param  buffer: Pointer to the buffer to store received data.
  * @param  length: Maximum length of the buffer.
  * @retval Number of bytes received.
  */
int32_t esp8266_io_recv(uint8_t* buffer, uint32_t length)
{
    uint32_t read_data = 0;

    while (length--)
    {
        uint32_t tick_start = HAL_GetTick();
        do
        {
            if (wifi_rx_buffer.head != wifi_rx_buffer.tail)
            {
                *buffer++ = wifi_rx_buffer.data[wifi_rx_buffer.head++];
                read_data++;

                if (wifi_rx_buffer.head >= RING_BUFFER_SIZE)
                {
                    wifi_rx_buffer.head = 0;
                }
                break;
            }
        } while ((HAL_GetTick() - tick_start) < DEFAULT_TIME_OUT);
    }

    return read_data;
}

/**
  * @brief  UART RX event callback for Idle line and partial DMA transfer detection.
  * @param  huart: Pointer to the UART handle.
  * @param  size: Number of bytes received in the latest transfer.
  * @retval None.
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
  if (huart == wifi_uart_handle)
  {
    static uint16_t tail_pos = 0;

    if (size > tail_pos){
      tail_pos = size;
      if (tail_pos != wifi_rx_buffer.head)
      {
        wifi_rx_buffer.tail = tail_pos;
      }
    }

    // Restart DMA reception
    HAL_UARTEx_ReceiveToIdle_DMA(wifi_uart_handle, &wifi_rx_buffer.data[wifi_rx_buffer.tail], RING_BUFFER_SIZE);
  }
}

/**
  * @brief  Rx Transfer completed callbacks.
  * @param  huart  Pointer to a UART_HandleTypeDef structure that contains
  *                the configuration information for the specified UART module.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{

}

/**
  * @brief  UART error callback.
  * @param  huart: Pointer to the UART handle.
  * @retval None.
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    esp8266_io_error_handler();
}

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Handle UART errors by deinitializing the interface.
  * @retval None.
  */
static void esp8266_io_error_handler(void)
{
    HAL_UART_DMAStop(wifi_uart_handle);
    HAL_UART_DeInit(wifi_uart_handle);

    while (1)
    {
    }
}
