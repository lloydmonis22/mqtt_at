 /*
 * esp8266_io.h
 *
 *  Created on: Dec 27, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#ifndef INC_ESP8266_IO_H_
#define INC_ESP8266_IO_H_

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef *wifi_uart_handle;
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DEFAULT_TIME_OUT                 1000 /* in ms */

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
int8_t esp8266_io_init(void);
void esp8266_io_deinit(void);
void io_buff_reset(void);
int8_t uart_dma_restart(void);


int8_t esp8266_io_send(uint8_t* Buffer, uint32_t Length);
int32_t esp8266_io_recv(uint8_t* Buffer, uint32_t Length);


#endif /* INC_ESP8266_IO_H_ */
