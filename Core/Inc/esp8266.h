/*
 * esp8266.h
 *
 *  Created on: Dec 27, 2024
 *      Author: Shreyas Acharya, BHARATI SOFTWARE
 */

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f4xx_hal.h"

/* Private define ------------------------------------------------------------*/
#define MAX_AT_CMD_SIZE         256
#define MAX_BUFFER_SIZE         (1024 * 8)
#define AT_OK_STRING            "OK\r\n"
#define AT_CONNECT_STRING       "CONNECT\r\n"
#define AT_IPD_OK_STRING        "OK\r\n\r\n"
#define AT_SEND_OK_STRING       "SEND OK\r\n"
#define AT_SEND_PROMPT_STRING   "OK\r\n\r\n>"
#define AT_ERROR_STRING         "ERROR\r\n"
#define AT_IPD_STRING           "+IPD,"

/* Exported types ------------------------------------------------------------*/
typedef enum {
    ESP8266_FALSE         = 0,
    ESP8266_TRUE          = 1
} esp8266_boolean;

typedef enum {
    ESP8266_OK                            = 0,
    ESP8266_ERROR                         = 1,
    ESP8266_BUSY                          = 2,
    ESP8266_ALREADY_CONNECTED             = 3,
    ESP8266_CONNECTION_CLOSED             = 4,
    ESP8266_TIMEOUT                       = 5,
    ESP8266_IO_ERROR                      = 6,
} esp8266_status_t;

typedef enum {
    ESP8266_ENCRYPTION_OPEN         = 0,
    ESP8266_ENCRYPTION_WEP          = 1,
    ESP8266_ENCRYPTION_WPA_PSK      = 2,
    ESP8266_ENCRYPTION_WPA2_PSK     = 3,
    ESP8266_ENCRYPTION_WPA_WPA2_PSK = 4,
} esp8266_encryption_t;

typedef enum {
    ESP8266_STATION_MODE      = 0,
    ESP8266_ACCESSPOINT_MODE  = 1,
    ESP8266_MIXED_MODE        = 2,
} esp8266_mode_t;

typedef enum {
    NORMAL_MODE      = 0,
    UNVARNISHED_MODE = 1
} esp8266_transfer_mode_t;

typedef enum {
    ESP8266_GOT_IP_STATUS       = 1,
    ESP8266_CONNECTED_STATUS    = 2,
    ESP8266_DISCONNECTED_STATUS = 3,
} esp8266_connection_status_t;

typedef enum {
    ESP8266_TCP_CONNECTION = 0,
    ESP8266_UDP_CONNECTION = 1,
} esp8266_connection_mode_t;

typedef enum {
    UDP_PEER_NO_CHANGE        = 0,
    UDP_PEER_CHANGE_ONCE      = 1,
    UDP_PEER_CHANGE_ALLOWED   = 2,
    UDP_PEER_CHANGE_INVALID   = -1,
} esp8266_connection_policy_t;

typedef struct {
  esp8266_connection_status_t    connection_status;
    esp8266_connection_mode_t    connection_type;
    esp8266_connection_policy_t  connection_mode;  /* For UDP connections only */
    uint8_t                      connection_id;
    uint8_t*                     ip_address;
    uint32_t                     port;
    uint32_t                     local_port;       /* For UDP connection only */
    esp8266_boolean              is_server;
} esp8266_connection_info_t;

typedef struct {
    uint8_t*                     ssid;
    uint8_t*                     password;
    uint16_t                   channel_id;
    esp8266_encryption_t  encryption_mode;
} esp8266_ap_config_t;

/* Exported functions ------------------------------------------------------- */
esp8266_status_t esp8266_init (void);
esp8266_status_t esp8266_deinit(void);
esp8266_status_t esp8266_reset(void);

esp8266_status_t esp8266_quit_ap(void);
esp8266_status_t esp8266_joint_ap(uint8_t* ssid, uint8_t* password);
esp8266_status_t esp8266_get_ip(esp8266_mode_t mode, uint8_t* ip_address);
esp8266_status_t esp8266_establish_connection(const esp8266_connection_info_t* connection_info);
esp8266_status_t esp8266_close_connection(const uint8_t channel_id);

esp8266_status_t esp8266_config_sntp(const char *ntp_server);
esp8266_status_t esp8266_get_sntp_time(void);
esp8266_status_t esp8266_mqtt_usercfg(const char *clientId, const char *username, const char *password);
esp8266_status_t esp8266_mqtt_connect(const char *endpoint, uint16_t port, uint8_t secure);
esp8266_status_t esp8266_mqtt_subscribe(const char *topic, uint8_t qos);
esp8266_status_t esp8266_mqtt_publish(const char *topic, const char *message, uint8_t qos, uint8_t retain);
esp8266_status_t catch_incoming_message(uint8_t* messageBuffer, uint32_t maxBufferLength, const uint8_t* token);
esp8266_status_t esp8266_send_data(uint8_t* pData, uint32_t length);
esp8266_status_t esp8266_recv_data(uint8_t* pData, uint32_t length, uint32_t* ret_length);


#endif /* INC_ESP8266_H_ */
