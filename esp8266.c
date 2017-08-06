#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "esp8266.h"

#define EVENT_POOL_SIZE 1

struct esp8266 {
  struct at_parser *parser;
  void (*eventHandler)(ESP_Event_t, void *user);
  void *userdata_eventhandler;
  ESP_Event_t eventpool[EVENT_POOL_SIZE];
};

struct esp8266 _esp8266 = {0};

struct esp8266* esp8266_get() {
  return &_esp8266;
}

void esp8266_register_event_handler(struct esp8266 *esp, void (*eventHandler)(ESP_Event_t, void *user), void *userdata) {
  esp->eventHandler = eventHandler;
  esp->userdata_eventhandler = userdata;
}

int esp8266_init(struct esp8266 *esp) {
  if (!esp || !esp->parser) {
    return 0;
  }

  at_send_data(esp->parser, "AT\r\n", 4);
  int resp = at_wait_for_response(esp->parser);

  if (resp != RESP_OK) {
    return 0;
  }
  return 1;
}

void esp8266_set_at_parser(struct esp8266 *esp, struct at_parser *p) {
  esp->parser = p;
}

int esp8266_response_parser(char *buff, int size, void *userdata) {
  struct esp8266 *esp = (struct esp8266 *)userdata;
  printf("GOT ESP: %s\n", buff);

  /* Basic response status handling */
  if (strncmp(buff, "OK", 2) == 0) {
    return RESP_OK;
  } else if (strncmp(buff, "no this fun", 11) == 0) {
    return 1;
  } else if (strncmp(buff, "ERROR", 5) == 0) {
    return RESP_ERROR;
  }

  if (buff[0] == '+') {
    if (strncmp(buff, "+CWLAP:", 7) == 0) {
      esp->eventpool[0].type = 1;
      esp->eventHandler(esp->eventpool[0], esp->userdata_eventhandler);
    }
  }
  return RESP_NONE;
}
