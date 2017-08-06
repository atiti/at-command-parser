#ifndef __PARSER_H__
#define __PARSER_H__

struct at_parser;

struct at_parser* at_get_parser();
// Callbacks to use for sending and receiving
int at_register_send(struct at_parser *p, int (*sendData)(char *, int, void *userdata), void *userdata);
int at_register_recv(struct at_parser *p, int (*recvData)(char *, int, void *userdata), void *userdata);

// Callback used for signaling events
int at_register_response_parser(struct at_parser *p, int (*responseParser)(char *, int, void *userdata), void *userdata);

// XXX: to remove?
int at_send_data(struct at_parser *p, char *buf, int len);
int at_wait_for_data(struct at_parser *p);
//int _at_parse_incoming(struct at_parser *p, int bytes, int *next);
int at_wait_for_response(struct at_parser *p);

#endif
