#ifndef __ESP8266_H__
#define __ESP8266_H__

typedef enum _ESP_Mode_t {
	ESP_Mode_STA = 0x01,                                /*!< ESP in station mode */
	ESP_Mode_AP = 0x02,                                 /*!< ESP as software Access Point mode */
	ESP_Mode_STA_AP = 0x03                              /*!< ESP in both modes */
} ESP_Mode_t;


/**
 * \brief           Transfer mode enumeration
 */
typedef enum _ESP_TransferMode_t {
    ESP_TransferMode_Normal = 0x00,                     /*!< Normal transfer mode of data packets */
    ESP_TransferMode_Transparent = 0x01                 /*!< UART<->WiFi transparent (passthrough) data mode */
} ESP_TransferMode_t;

/**
 * \brief           Security settings for wifi network
 */
typedef enum _ESP_Ecn_t {
	ESP_Ecn_OPEN = 0x00,                                /*!< Wifi is open */
	ESP_Ecn_WEP = 0x01,                                 /*!< Wired Equivalent Privacy option for wifi security. \note  This mode can't be used when setting up ESP8266 wifi */
	ESP_Ecn_WPA_PSK = 0x02,                             /*!< Wi-Fi Protected Access */
	ESP_Ecn_WPA2_PSK = 0x03,                            /*!< Wi-Fi Protected Access 2 */
	ESP_Ecn_WPA_WPA2_PSK = 0x04,                        /*!< Wi-Fi Protected Access with both modes */
} ESP_Ecn_t;

/**
 * \brief           Sleep mode enumeration
 */
typedef enum _ESP_SleepMode_t {
	ESP_SleepMode_Disable = 0x00,                       /*!< Sleep mode disabled */
	ESP_SleepMode_Light = 0x01,                         /*!< Light sleep mode */
	ESP_SleepMode_Modem = 0x02                          /*!< Model sleep mode */
} ESP_SleepMode_t;

/**
 * \brief           Connection type
 */
typedef enum _ESP_CONN_Type_t {
	ESP_CONN_Type_TCP = 0x00,                           /*!< Connection type is TCP */
	ESP_CONN_Type_UDP = 0x01,                           /*!< Connection type is UDP */
	ESP_CONN_Type_SSL = 0x02                            /*!< Connection type is SSL */
} ESP_CONN_Type_t;

typedef enum _ESP_CONN_Status_t {
  RESP_NONE,
  RESP_OK = 0x01,
	RESP_ERROR,
	RESP_BRACKET,
	RESP_READY,
	RESP_CONNECT_OK,
	RESP_CONNECT_FAIL,
	RESP_CONNECT_ALREADY,
	RESP_CLOSE_OK,
	RESP_SEND_OK,
	RESP_SEND_FAIL,
	RESP_WIFI_CONNECTED,
	RESP_WIFI_DISCONNECTED,
	RESP_WIFI_GOT_IP
} ESP_CONN_Status_t;

typedef enum _ESP_EventType_t {
  NONE,
  WIFI_SCAN_RESULTS = 0x01
} ESP_EventType_t;

typedef struct _ESP_Event_t {
  ESP_EventType_t type;
  int used;
} ESP_Event_t;


struct esp8266;

struct esp8266* esp8266_get();
int esp8266_init(struct esp8266 *esp);
void esp8266_register_event_handler(struct esp8266 *esp, void (*eventHandler)(ESP_Event_t, void *user), void *userdata);
void esp8266_set_at_parser(struct esp8266 *esp, struct at_parser *p);

/* This is the ESP specific parser callback required for the generic AT parser */
int esp8266_response_parser(char *buff, int size, void *userdata);


#endif
