#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

#define RX_BUFFER_SIZE 64

struct at_parser {
  int rx_offset;
  char rxbuffer[RX_BUFFER_SIZE];
  int (*sendData)(char *, int, void *);
  void *userdata_send;
  int (*recvData)(char *, int, void *);
  void *userdata_recv;

  int (*responseParser)(char *, int, void *);
  void *userdata_respparser;
};

struct at_parser _parser = {0};

struct at_parser* at_get_parser() {
  return &_parser;
}

int at_register_send(struct at_parser *p, int (*sendData)(char *, int, void *), void *userdata) {
  p->sendData = sendData;
  p->userdata_send = userdata;
  return 1;
}

int at_register_recv(struct at_parser *p, int (*recvData)(char *, int, void *), void *userdata) {
  p->recvData = recvData;
  p->userdata_recv = userdata;
  p->rx_offset = 0;
  return 1;
}

int at_register_response_parser(struct at_parser *p, int (*responseParser)(char *, int, void *), void *userdata) {
  p->responseParser = responseParser;
  p->userdata_respparser = userdata;
  return 1;
}

int at_send_data(struct at_parser *p, char *buff, int len) {
  if (!p || !p->sendData) {
    return -1;
  }
  if (!len) return 0;

  p->sendData(buff, len, p->userdata_send);
  return len;
}

int at_wait_for_data(struct at_parser *p) {
  if (!p || !p->recvData) {
    return -1;
  }

  char *buffer = p->rxbuffer + p->rx_offset;
  int read_size = RX_BUFFER_SIZE - p->rx_offset - 1;
  int num_bytes = p->recvData(buffer, read_size, p->userdata_recv);
  return num_bytes;
}

int at_parse_line(struct at_parser *p, int offset) {
  int len = offset;
  char *buffer = p->rxbuffer + len;
  while ( len < RX_BUFFER_SIZE && *buffer) {
    char c = *buffer++;
    if (c == '\r') {
      p->rxbuffer[len] = 0;
      if (p->rxbuffer[len+1] == '\n') {
        p->rxbuffer[len+1] = 0;
        return len+1;
      } else {
        return len;
      }
    }
    if (c == '\0') {
      break;
    }
    len++;
  }

  /** Handle the case where we've got a fragmented line
      Most likely our RX buffer is too small for it
  */
  if (len > offset) {
    memmove(p->rxbuffer, p->rxbuffer + offset, RX_BUFFER_SIZE - offset - 1);
    p->rx_offset = RX_BUFFER_SIZE - offset - 1;
  } else {
    p->rx_offset = 0;
  }
  //printf("***** len %d offset %d rxoffset %d\n", len, offset, p->rx_offset);
  return -1;
}

int at_parse_incoming(struct at_parser *p, int bytes, int *next) {
  int offset = *next;
  int ret = -1;

  if (bytes == 0) {
    return -1;
  }

  ret = at_parse_line(p, offset);
  if (ret != -1) {
    *next = ret + 1;
  } else {
    return -1;
  }
  return offset;
}

int at_wait_for_response(struct at_parser *p) {
  int response = 0;
  int next = 0;
  int offset = -1;

  while (!response) {
    offset = -1;
    next = 0;
    int num_bytes = at_wait_for_data(p);
    do {
      offset = at_parse_incoming(p, num_bytes, &next);
      if (offset != -1) {
        char *line = p->rxbuffer + offset;
        /* Process response */
        if (line[0] == '\r' ||
            line[0] == '\n' ||
            line[0] == '\0') {
          continue;
        }
        printf("Line: %s\n", line);

        if (!p->responseParser) {
          if (strncmp(line, "OK", 2) == 0) {
            response = 1;
          } else if (strncmp(line, "no this fun", 11) == 0) {
            response = 1;
          } else if (strncmp(line, "ERROR", 5) == 0) {
            response = 1;
          }
        } else {
          response = p->responseParser(line, strlen(line), p->userdata_respparser);
        }
      }
    } while (offset != -1);
  }

  return response;
}
