/*
 * esp8266.c
 *
 *  Created on: Dec 27, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#include "esp8266.h"
#include "esp8266_io.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define CMD_TERMINATOR "\r\n"
#define RESPONSE_OK "OK"

static char at_cmd[MAX_AT_CMD_SIZE];
static char rx_buffer[MAX_BUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
static esp8266_status_t send_at_cmd(uint8_t* cmd, uint32_t Length, const uint8_t* Token);
static esp8266_status_t recv_data(uint8_t* Buffer, uint32_t Length, uint32_t* retLength);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief   Initialize the esp8266 module.
  *          IT intitalize the IO to communicate between the MCU and the module, then
  *          test that the modules is working using some basic AT commands.
  *          in case of success the string "OK" is returned inside otherwise
  *          it is an error.
  * @param   None
  * @retval  ESP8266_OK on success, ESP8266_ERROR otherwise.
  */
esp8266_status_t esp8266_init(void)
{
  esp8266_status_t ret;

  /* Configuration the IO low layer */
  if (esp8266_io_init() < 0)
  {
    return ESP8266_ERROR;
  }

  /* Disable the Echo mode */
#if 1
  /* Construct the command */
  memset (at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "ATE0%c%c", '\r', '\n');

  /* Send the command */
  ret = send_at_cmd((uint8_t* )at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);

  /* Exit in case of error */
  if (ret !=  ESP8266_OK)
  {
    return ESP8266_ERROR;
  }
#endif
  /* Setup the module in Station Mode*/

  /* Construct the command */
  memset (at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+CWMODE=1%c%c", '\r', '\n');

  /* Send the command */
  ret = send_at_cmd((uint8_t* )at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);

  return ret;
}

/**
  * @brief   Deinitialize the ESP8266 module.
  * @details Restarts the module  and stop the IO. AT command can't be executed
             unless the module is reinitialized.
  * @param   None
  * @retval  ESP8266_OK on success, ESP8266_ERROR otherwise.
  */
esp8266_status_t esp8266_deinit(void)
{
  esp8266_status_t ret;

  /* Construct the command */
  sprintf((char *)at_cmd, "AT+RST%c%c", '\r', '\n');

  /* Send the command */
  ret = send_at_cmd((uint8_t* )at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);

  /* Free resources used by the module */
  esp8266_io_deinit();

  return ret;
}


/**
  * @brief  Restarts the esp8266 module.
  * @param  None
  * @retval ESP8266_OK on success, ESP8266_ERROR otherwise.
  */
esp8266_status_t esp8266_reset(void)
{
  esp8266_status_t ret;

  /* Construct the command */
  memset (at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+RST%c%c", '\r', '\n');

  /* Send the command */
  ret = send_at_cmd((uint8_t* )at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);

  return ret;
}

/**
  * @brief  Join an Access point.
  * @param  Ssid: the access point id.
  * @param  Password the Access point password.
  * @retval returns ESP8266_AT_COMMAND_OK on success and ESP8266_AT_COMMAND_ERROR otherwise.
  */
esp8266_status_t esp8266_joint_ap(uint8_t* Ssid, uint8_t* Password)
{
  esp8266_status_t ret;

  /* List all the available Access points first
   then check whether the specified 'ssid' exists among them or not.*/
  memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+CWJAP=\"%s\",\"%s\"%c%c", Ssid, Password, '\r', '\n');

  /* Send the command */
  ret = send_at_cmd((uint8_t*)at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);

#if 0
  /* Disable multiple connection by default. */

  /* Construct the command */
  memset (at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+CIPMUX=0%c%c", '\r', '\n');

  /* Send the command */
  ret = send_at_cmd((uint8_t*)at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);
#endif
  return ret;
}

/**
  * @brief  Quit an Access point if any.
  * @param  None
  * @retval returns ESP8266_AT_COMMAND_OK on success and ESP8266_AT_COMMAND_ERROR otherwise.
  */
esp8266_status_t esp8266_quit_ap(void)
{
  esp8266_status_t ret;

  /* Construct the CWQAP command */
  memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+CWQAP%c%c", '\r', '\n');

  /* Send the command */
  ret = send_at_cmd((uint8_t*)at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);

  return ret;
}

/**
  * @brief  Get the IP address for the esp8266 in Station mode.
  * @param  Mode: a ESP8266_ModeTypeDef to choose the Station or AccessPoint mode.
                 only the Station Mode is supported.
  * @param  IpAddress buffer to contain the IP address.
  * @retval returns ESP8266_OK on success and ESP8266_ERROR otherwise
  */
esp8266_status_t esp8266_get_ip(esp8266_mode_t Mode, uint8_t* IpAddress)
{
  esp8266_status_t ret = ESP8266_OK;
  char *Token, *temp;

  /* Initialize the IP address and command fields */
  strcpy((char *)IpAddress, "0.0.0.0");
  memset(at_cmd, '\0', MAX_AT_CMD_SIZE);

  /* Construct the CIFSR command */
  sprintf((char *)at_cmd, "AT+CIFSR%c%c", '\r', '\n');

  /* Send the CIFSR command */
  ret = send_at_cmd((uint8_t* )at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);

  /* If ESP8266_OK is returned it means the IP Adress inside the rx_buffer
     has already been read */
  if ( ret == ESP8266_OK)
  {
    /* The IpAddress for the Station Mode is returned in the format
     ' STAIP,"ip_address" '
      look for the token "STAIP," , then read the ip address located
      between two double quotes */
    Token = strstr((char *)rx_buffer, "STAIP,");
    Token+=7;

    temp = strstr(Token, "\"");
    *temp = '\0';

    /* Get the IP address value */
    strcpy((char *)IpAddress, Token);
  }

  return ret;
}

/**
  * @brief  Establish a network connection.
  * @param  Connection_info a pointer to a ESP8266_ConnectionInfoTypeDef struct containing the connection info.
  * @retval returns ESP8266_AT_COMMAND_OK on success and ESP8266_AT_COMMAND_ERROR otherwise.
  */
esp8266_status_t esp8266_establish_connection(const esp8266_connection_info_t* connection_info)
{
  esp8266_status_t ret;

  /* Check the connection mode */
  if (connection_info->is_server)
  {
  /* Server mode not supported for this version yet */
    return ESP8266_ERROR;
  }

  /* Construct the CIPSTART command */
  memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+CIPSTART=\"TCP\",\"%s\",%lu%c%c", (char *)connection_info->ip_address, connection_info->port,'\r', '\n');

  /* Send the CIPSTART command */
  ret = send_at_cmd((uint8_t*)at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_CONNECT_STRING);

  return ret;
}

/**
  * @brief   Close a network connection.
  * @details Use the ALL_CONNECTION_ID to close all connections.
  * @param   Channel_id the channel ID of the connection to close.
  * @retval  returns ESP8266_AT_COMMAND_OK on success and ESP8266_AT_COMMAND_ERROR otherwise.
  */
esp8266_status_t esp8266_close_connection(const uint8_t channel_id)
{
  /* Working with a single connection, no channel_id is required */
  esp8266_status_t ret;

  /* Construct the CIPCLOSE command */
  memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+CIPCLOSE%c%c", '\r', '\n');

  /* Send the CIPCLOSE command */
  ret = send_at_cmd((uint8_t* )at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);

  return ret;
}

/* === Added Functions for AWS IoT MQTT and SNTP Commands === */

/**
  * @brief  Configure the SNTP client with a specified NTP server.
  * @param  ntp_server: string containing the NTP server address (e.g., "pool.ntp.org").
  * @retval ESP8266_OK on success, ESP8266_ERROR otherwise.
  */
esp8266_status_t esp8266_config_sntp(const char *ntp_server)
{
  esp8266_status_t ret;
  memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+CIPSNTPCFG=1,8,\"%s\"%c%c", ntp_server, '\r', '\n');
  ret = send_at_cmd((uint8_t*)at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);
  return ret;
}

/**
  * @brief  Query the SNTP time from the module.
  * @retval ESP8266_OK on success, ESP8266_ERROR otherwise.
  */
esp8266_status_t esp8266_get_sntp_time(void)
{
  esp8266_status_t ret;
  memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+CIPSNTPTIME?%c%c", '\r', '\n');
  ret = send_at_cmd((uint8_t*)at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);
  return ret;
}

/**
  * @brief  Configure MQTT client parameters.
  * @param  clientId: MQTT client ID (e.g., "esp32").
  * @param  username: MQTT username (e.g., "espressif").
  * @param  password: MQTT password (e.g., "1234567890").
  * @retval ESP8266_OK on success, ESP8266_ERROR otherwise.
  */
esp8266_status_t esp8266_mqtt_usercfg(const char *clientId, const char *username, const char *password)
{
  esp8266_status_t ret;
  memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+MQTTUSERCFG=0,5,\"%s\",\"%s\",\"%s\",0,0,\"\"%c%c", clientId, username, password, '\r', '\n');
  ret = send_at_cmd((uint8_t*)at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);
  return ret;
}

/**
  * @brief  Connect to the MQTT broker.
  * @param  endpoint: MQTT broker endpoint (e.g., your AWS IoT endpoint).
  * @param  port: Port number (e.g., 8883).
  * @param  secure: Use secure connection (1 for secure, 0 for non-secure).
  * @retval ESP8266_OK on success, ESP8266_ERROR otherwise.
  */
esp8266_status_t esp8266_mqtt_connect(const char *endpoint, uint16_t port, uint8_t secure)
{
  esp8266_status_t ret;
  memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+MQTTCONN=0,\"%s\",%u,%u%c%c", endpoint, port, secure, '\r', '\n');
  ret = send_at_cmd((uint8_t*)at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);
  return ret;
}

/**
  * @brief  Subscribe to an MQTT topic.
  * @param  topic: MQTT topic to subscribe to (e.g., "topic/esp32at").
  * @param  qos: Quality of Service level (typically 1).
  * @retval ESP8266_OK on success, ESP8266_ERROR otherwise.
  */
esp8266_status_t esp8266_mqtt_subscribe(const char *topic, uint8_t qos)
{
  esp8266_status_t ret;
  memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+MQTTSUB=0,\"%s\",%u%c%c", topic, qos, '\r', '\n');
  ret = send_at_cmd((uint8_t*)at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);
  return ret;
}

/**
  * @brief  Publish a message to an MQTT topic.
  * @param  topic: MQTT topic to publish to (e.g., "topic/esp32at").
  * @param  message: The message to publish (e.g., "hello aws!").
  * @param  qos: Quality of Service level (typically 1).
  * @param  retain: Retain flag (0 or 1).
  * @retval ESP8266_OK on success, ESP8266_ERROR otherwise.
  */
esp8266_status_t esp8266_mqtt_publish(const char *topic, const char *message, uint8_t qos, uint8_t retain)
{
  esp8266_status_t ret;

  memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
  sprintf((char *)at_cmd, "AT+MQTTPUB=0,\"%s\",\"%s\",%u,%u%c%c", topic, message, qos, retain, '\r', '\n');
  ret = send_at_cmd((uint8_t*)at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_OK_STRING);
  return ret;
}

/* === End of Added Functions === */

/**
  * @brief  Send data over the wifi connection.
  * @param  Buffer: the buffer to send
  * @param  Length: the Buffer's data size.
  * @retval returns ESP8266_OK on success and ESP8266_ERROR otherwise.
  */
esp8266_status_t esp8266_send_data(uint8_t* Buffer, uint32_t Length)
{
  //uart_dma_restart();
  esp8266_status_t ret = ESP8266_OK;

  if (Buffer != NULL)
  {
    //uint32_t tickStart;
    /* Construct the CIPSEND command */
    memset(at_cmd, '\0', MAX_AT_CMD_SIZE);
    sprintf((char *)at_cmd, "AT+CIPSEND=%lu%c%c", Length, '\r', '\n');

    /* The CIPSEND command doesn't have a return command
       until the data is actually sent. Thus we check here whether
       we got the '>' prompt or not. */
    ret = send_at_cmd((uint8_t* )at_cmd, strlen((char *)at_cmd), (uint8_t*)AT_SEND_PROMPT_STRING);

    /* return Error */
    if (ret != ESP8266_OK)
  {
      return ESP8266_ERROR;
  }

   /* Wait before sending data. */
//    tickStart = HAL_GetTick();
//    while (HAL_GetTick() - tickStart < 500)
//    {
//    }


  /* Send the data */
  ret = send_at_cmd(Buffer, Length, (uint8_t*)AT_SEND_OK_STRING);
  }

  return ret;
}


/**
  * @brief  receive data over the wifi connection.
  * @param  pData the buffer to fill will the received data.
  * @param  Length the maximum data size to receive.
  * @param retLength the actual data received.
  * @retval returns ESP8266_OK on success and ESP8266_ERROR otherwise.
  */
esp8266_status_t esp8266_recv_data(uint8_t* pData, uint32_t Length, uint32_t* retLength)
{
  esp8266_status_t ret;

  /* Receive the data from the host */
  ret = recv_data(pData, Length, retLength);

  return ret;
}

/**
  * @brief  Run the AT command
  * @param  cmd the buffer to fill will the received data.
  * @param  Length the maximum data size to receive.
  * @param  Token the expected output if command runs successfully
  * @retval returns ESP8266_OK on success and ESP8266_ERROR otherwise.
  */
static esp8266_status_t send_at_cmd(uint8_t* cmd, uint32_t Length, const uint8_t* Token)
{
  uint32_t idx = 0;
  uint8_t RxChar;

  /* Reset the Rx buffer to make sure no previous data exist */
  memset(rx_buffer, '\0', MAX_BUFFER_SIZE);

  /* Send the command */
  if (esp8266_io_send(cmd, Length) < 0)
  {
    return ESP8266_ERROR;
  }

  /* Wait for reception */
  while (1)
  {
  /* Wait to receive data */
    if (esp8266_io_recv(&RxChar, 1) != 0)
    {
      rx_buffer[idx++] = RxChar;
    }
    else
    {
      break;
    }

  /* Check that max buffer size has not been reached */
    if (idx == MAX_BUFFER_SIZE)
    {
      break;
    }

    /* Extract the Token */
    if (strstr((char *)rx_buffer, (char *)Token) != NULL)
    {
      return ESP8266_OK;
    }

  /* Check if the message contains error code */
    if (strstr((char *)rx_buffer, AT_ERROR_STRING) != NULL)
    {
      return ESP8266_ERROR;
    }
  }

  return ESP8266_ERROR;
}

/**
  * @brief  Receive data from the WiFi module
  * @param  Buffer The buffer where to fill the received data
  * @param  Length the maximum data size to receive.
  * @param  retLength Length of received data
  * @retval returns ESP8266_OK on success and ESP8266_ERROR otherwise.
  */
static esp8266_status_t recv_data(uint8_t* Buffer, uint32_t Length, uint32_t* retLength)
{
  uint8_t RxChar;
  uint32_t idx = 0;
  uint8_t LengthString[4];
  uint32_t LengthValue;
  uint8_t i = 0;
  esp8266_boolean newChunk  = ESP8266_FALSE;

  /* Reset the reception data length */
  *retLength = 0;

  /* Reset the reception buffer */
  memset(rx_buffer, '\0', MAX_BUFFER_SIZE);

  /* When reading data over a wifi connection the esp8266
     splits it into chunks of 1460 bytes maximum each, each chunk is preceded
     by the string "+IPD,<chunk_size>:". Thus to get the actual data we need to:
       - Receive data until getting the "+IPD," token, a new chunk is marked.
       - Extract the 'chunk_size' then read the next 'chunk_size' bytes as actual data
       - Mark end of the chunk.
       - Repeat steps above until no more data is available. */
  while (1)
  {
    if (esp8266_io_recv(&RxChar, 1) != 0)
    {
      /* The data chunk starts with +IPD,<chunk length>: */
      if ( newChunk == ESP8266_TRUE)
      {
        /* Read the next lengthValue bytes as part from the actual data. */
        if (LengthValue--)
        {
          *Buffer++ = RxChar;
          (*retLength)++;
        }
        else
        {
           /* Clear the buffer as the new chunk has ended. */
           newChunk = ESP8266_FALSE;
           memset(rx_buffer, '\0', MAX_BUFFER_SIZE);
           idx = 0;
        }
      }
      rx_buffer[idx++] = RxChar;
    }
    else
    {
     /* Errors while reading return an error. */
      if ((newChunk == ESP8266_TRUE) && (LengthValue != 0))
      {
        return ESP8266_ERROR;
      }
      else
      {
        break;
      }
    }

    if (idx == MAX_BUFFER_SIZE)
    {
      /* In case of Buffer overflow, return error */
      if ((newChunk == ESP8266_TRUE) && (LengthValue != 0))
      {
        return ESP8266_ERROR;
      }
      else
      {
        break;
      }
    }

    /* When a new chunk is met, extract its size */
    if ((strstr((char *)rx_buffer, AT_IPD_STRING) != NULL) && (newChunk == ESP8266_FALSE))
    {
      i = 0;
      memset(LengthString, '\0', 4);
      do
      {
        esp8266_io_recv(&RxChar, 1);
        LengthString[i++] = RxChar;
      }
      while(RxChar != ':');

    /* Get the buffer length */
      LengthValue = atoi((char *)LengthString);
      newChunk = ESP8266_TRUE;
    }

    /* Check if message contains error code */
    if (strstr((char *)rx_buffer, AT_ERROR_STRING) != NULL)
    {
      return ESP8266_ERROR;
    }

    /* Check for the chunk end */
    if (strstr((char *)rx_buffer, AT_IPD_OK_STRING) != NULL)
    {
      newChunk = ESP8266_FALSE;
    }
  }

  return ESP8266_OK;
}

/**
  * @brief  Process incoming data until a specified token is detected.
  * @param  messageBuffer: Buffer to store the incoming message.
  * @param  maxBufferLength: Maximum length of the messageBuffer.
  * @param  token: The expected token string that indicates the end of a valid message.
  * @retval ESP8266_OK if the token is detected, ESP8266_ERROR otherwise.
  */
esp8266_status_t catch_incoming_message(uint8_t* messageBuffer, uint32_t maxBufferLength, const uint8_t* token)
{
    uint32_t idx = 0;
    uint8_t RxChar;

    /* Clear the messageBuffer */
    memset(messageBuffer, '\0', maxBufferLength);

    /* Continuously receive data until no more data is available or the token is found */
    while (1)
    {
        /* Attempt to receive one byte (non-blocking) */
        if (esp8266_io_recv(&RxChar, 1) != 0)
        {
            messageBuffer[idx++] = RxChar;
        }
        else
        {
            /* No more data available at this moment, exit the loop */
            break;
        }

        /* Prevent buffer overflow */
        if (idx == maxBufferLength)
        {
            break;
        }

        /* Check if the expected token is in the accumulated message */
        if (strstr((char *)messageBuffer, (char *)token) != NULL)
        {
            return ESP8266_OK;
        }

        /* Also check if an error string is present */
        if (strstr((char *)messageBuffer, AT_ERROR_STRING) != NULL)
        {
            return ESP8266_ERROR;
        }
    }

    /* If we exit the loop without finding the token, return an error */
    return ESP8266_ERROR;
}
