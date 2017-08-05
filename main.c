#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "parser.h"

int fd = 0;

int set_interface_attribs(int fd, int speed, int parity) {
  struct termios tty;
  memset(&tty, 0, sizeof(tty));

  if (tcgetattr(fd, &tty) != 0) {
    fprintf(stderr, "Error %d from tcgetattr\n", errno);
    return -1;
  }

  cfsetospeed(&tty, speed);
  cfsetispeed(&tty, speed);

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;    // 8-bit chars
  tty.c_iflag &= ~IGNBRK;                        // Disable IGNBRK for mismatch speed tests
  tty.c_lflag = 0;                               // No signaling chars, no echo, no canonical processing
  tty.c_oflag = 0;                               // No remapping, no delays
  tty.c_cc[VMIN] = 0;                            // Read doesn't block
  tty.c_cc[VTIME] = 10;                           // 0.5s read timeout
  tty.c_iflag &= ~(IXON | IXOFF | IXANY);        // Shut off xoff/xon
  tty.c_cflag |= (CLOCAL | CREAD);               // Ignore modem controls, enable read
  tty.c_cflag &= ~(PARENB | PARODD);             // Shut off parity
  tty.c_cflag |= parity;                         // Set parity
  tty.c_cflag &= ~CSTOPB;                        // No stop bit
  tty.c_cflag &= ~CRTSCTS;                       // No hw flow control

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    fprintf(stderr, "Error setting attribs: %d\n", errno);
    return -1;
  }
  return 0;
}

void set_blocking(int fd, int should_block) {
  struct termios tty;
  memset(&tty, 0, sizeof(tty));
  if (tcgetattr(fd, &tty) != 0) {
    fprintf(stderr, "Error %d from tgetattr\n", errno);
    return;
  }

  tty.c_cc[VMIN] = should_block ? 1 : 0;
  tty.c_cc[VTIME] = 10;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    fprintf(stderr, "Error setting attribs: %d\n", errno);
  }
}

// HAL functions for sending and receiving
int sendSerial(char *buf, int size, void *userdata) {
  printf("Sending (%d): %s\n", size, buf);
  int num_written = write(fd, buf, size);
  tcdrain(fd);
  return num_written;
}

int recvSerial(char *buf, int size, void *userdata) {
  memset(buf, 0, size);
  int rdlen = read(fd, buf, size);
//  for(int i = 0; i < rdlen; i++) {
//    printf("%c[%x] ", buf[i], buf[i]);
//  }
//  printf("\n");

//  if (rdlen) printf("Received (%d max size %d): %s\n", rdlen, size, buf);
  return rdlen;
}

int main() {
  char *portname = "/dev/cu.usbserial-A700dYB2";
  fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);

  if (fd < 0) {
    fprintf(stderr, "Unable to open port %d: %s\n", errno, strerror(errno));
    return -1;
  }

  set_interface_attribs(fd, B115200, 0);
  set_blocking(fd, 0);


  struct at_parser *p = at_get_parser();
  at_register_send(p, sendSerial, NULL);
  at_register_recv(p, recvSerial, NULL);
  //at_register_callback(p);

  // Start by reading off everything we have in the pipeline
  at_send_data(p, "AT\r\n", 4);
  at_wait_for_response(p);

  at_send_data(p, "ATE0\r\n", 6);
  at_wait_for_response(p);

  at_send_data(p, "AT+GMR\r\n", 8);
  at_wait_for_response(p);

  at_send_data(p, "AT+CWMODE?\r\n", 12);
  at_wait_for_response(p);

  at_send_data(p, "AT+CWJAP?\r\n", 11);
  at_wait_for_response(p);

  at_send_data(p, "AT+CWLAP\r\n", 10);
  at_wait_for_response(p);

  close(fd);
  return 0;
}
