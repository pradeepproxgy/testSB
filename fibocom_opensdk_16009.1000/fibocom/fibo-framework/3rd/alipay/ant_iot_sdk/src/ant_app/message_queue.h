#ifndef ALIPAY_IOT_MESSAGE_QUEUE_H
#define ALIPAY_IOT_MESSAGE_QUEUE_H
#include "ant_kal.h"
#ifdef __cplusplus
extern "C" {
#endif
typedef struct message_send_s {
    struct message_send_s *next;
    void *msg;
} message_send_t;

typedef struct {
    void *memory;
    ant_bool *used;
    message_send_t *pending;
    ant_size elem_size;
    ant_u32 elem_num;
} message_elem_t;

typedef struct {
    ant_sem msg_sem;
    ant_sem claim_sem;
    ant_mutex lock;
    message_elem_t elem;
} message_queue_t;

message_queue_t * message_queue_create(ant_size elem_size, ant_u32 elem_num);
void message_queue_destroy(message_queue_t *queue);
void message_queue_send(message_queue_t *queue, void *message, int is_front);
ant_s32 message_queue_recv(message_queue_t *queue, void *message, ant_s32 time_out);

#ifdef __cplusplus
}
#endif
#endif