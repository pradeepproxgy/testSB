#ifndef OPENCPU_API_OC_HEARTBEAT
#define OPENCPU_API_OC_HEARTBEAT


bool fibo_heartbeat_param_set(char *serverip, uint16_t port,  uint16_t time, char *hexdata);
bool fibo_enable_heartbeat(uint8_t cid,  uint8_t nsim, int32_t mode, int32_t protocol);
bool fibo_set_heartbeat_reconnect_timer(
    uint16_t time1, uint16_t time2,uint16_t time3,uint16_t time4,uint16_t time5,uint16_t time6);

#endif