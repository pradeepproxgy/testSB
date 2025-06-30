#ifndef A29CDF01_FDB1_4F13_8705_23DBB065C7A9
#define A29CDF01_FDB1_4F13_8705_23DBB065C7A9

#include <stdint.h>

/**
 * create a message queue
 *
 * \param msg_count maximum message count can be hold in queue
 * \param msg_size  size of each message in bytes
 * \return
 *      - message queue pointer
 *      - NULL on invalid parameter or out of memory
 */
int ffw_msg_queue_create(uint32_t msg_count, uint32_t msg_size, void **mqout);

/**
 * delete a message queue
 *
 * When \p mq is NULL, nothing will be done, just as \p free.
 *
 * \param mq    message queue pointer
 */
int ffw_msg_queue_delete(void *mq);

/**
 * put a message to message queue
 *
 * \p msg should hold content size the same as \p msg_size specified at
 * \p osiMessageQueueCreate.
 *
 * After put, the content of \p msg will be copied to message queue.
 *
 * When \p mq is full, it will be blocked until there are rooms.
 *
 * This can be called in ISR. And in ISR, it will return false when queue
 * is full, and not wait.
 *
 * \param mq    message queue pointer
 * \param msg   mesage pointer
 * \return
 *      - FFW_R_SUCCESS on success
 *      - FFW_R_FAIL oninvalid parameter
 */
int ffw_msg_queue_put(void *mq, const void *msg);

/**
 * put a message to message queue with timeout
 *
 * This can be called in ISR. And in ISR, \p timeout will be ignored.
 *
 * \param mq        message queue pointer
 * \param msg       mesage pointer
 * \param timeout   timeout in milliseconds
 * \return
 *      - FFW_R_SUCCESS on success
 *      - FFW_R_FAIL oninvalid parameter or timeout
 */
int ffw_msg_queue_try_put(void *mq, const void *msg, uint32_t timeout);


/**
 * put a message to message queue front
 *
 * \p msg should hold content size the same as \p msg_size specified at
 * \p osiMessageQueueCreate.
 *
 * After put, the content of \p msg will be copied to message queue.
 *
 * When \p mq is full, it will be blocked until there are rooms.
 *
 * \param mq    message queue pointer
 * \param msg   mesage pointer
 * \return
 *      - FFW_R_SUCCESS on success
 *      - FFW_R_FAIL oninvalid parameter
 */
int ffw_msg_queue_put_front(void *mq, const void *msg);


/**
 * put a message to message queue front with timeout
 *
 * This can be called in ISR. And in ISR, \p timeout must be 0.
 *
 * \param mq        message queue pointer
 * \param msg       mesage pointer
 * \param timeout   timeout in milliseconds
 * \return
 *      - FFW_R_SUCCESS on success
 *      - FFW_R_FAIL oninvalid parameter or timeout
 */
int ffw_msg_queue_try_put_front(void *mq, const void *msg, uint32_t timeout);


/**
* @brief 重置
* 
* @param mq 
* @param msg 
* @param timeout 
* @return int 
 */
int ffw_msg_queue_reset(void *mq);

/**
 * get a message to message queue
 *
 * \p msg should be able tp hold content size of \p msg_size specified at
 * \p osiMessageQueueCreate.
 *
 * After get, the content of message will be copied to \p msg.
 *
 * When \p mq is empty, it will be blocked until there are messages.
 *
 * This can be called in ISR. And in ISR, it will return false when queue
 * is empty, and not wait.
 *
 * \param mq    message queue pointer
 * \param msg   mesage pointer
 * \return
 *      - FFW_R_SUCCESS on success
 *      - FFW_R_FAIL oninvalid parameter
 */
int ffw_msg_queue_get(void *mq, void *msg);

/**
 * get a message to message queue with timeout
 *
 * This can be called in ISR. And in ISR, \p timeout will be ignored.
 *
 * \param mq        message queue pointer
 * \param msg       mesage pointer
 * \param timeout   timeout in milliseconds
 * \return
 *      - FFW_R_SUCCESS on success
 *      - FFW_R_FAIL oninvalid parameter or timeout
 */
int ffw_msg_queue_try_get(void *mq, void *msg, uint32_t timeout);

/**
 * get the pending message count in message queue
 *
 * It can be called in both thread context and ISR context.
 *
 * \param mq        message queue pointer
 * \return
 *      - the pending message count in message queue
 *      - 0 on invalid parameter
 */
uint32_t ffw_msg_queue_pending_count(void *mq);

/**
 * get the space of message count in message queue
 *
 * It can be called in both thread context and ISR context.
 *
 * \param mq        message queue pointer
 * \return
 *      - the space of message count in message queue
 *      - 0 on invalid parameter
 */
uint32_t ffw_msg_queue_space_count(void *mq);

#endif /* A29CDF01_FDB1_4F13_8705_23DBB065C7A9 */
