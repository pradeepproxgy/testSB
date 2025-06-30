#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "ffw_log.h"
#include "ffw_rat.h"



/**
 * @brief at+cereg?的上报
 * 
 * @param sStatus 网络信息的上报结构体
 * @param stat 网络状态,同sStatus.nStatus
 * @param reportN cereg是否上报标志 正常情况为true
 * @param engine at引擎
 * @param ffw_ceregreg cereg set的值
 * @param nSim sim id
 * @param nCurrRat 当前制式
 * @param sim_is_locked 是否锁卡
 * @return uint8_t
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */

uint8_t ffw_ceregrespond(ffw_nw_status_info_t *sStatus, uint8_t stat, bool reportN, void *engine, uint8_t ffw_ceregreg,uint8_t nSim,uint8_t nCurrRat, bool sim_is_locked );

/**
 * @brief cereg主动上报
 * 
 * @param sStatus 网络信息的上报结构体
 * @param stat 网络状态,同sStatus.nStatus
 * @param nSim sim id
 * @param ffw_ceregreg cereg set的值
 * @return uint8_t
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */

uint8_t ffw_atceregrespond(ffw_nw_status_info_t *sStatus, uint8_t stat, uint8_t nSim,uint8_t ffw_ceregreg);

/**
 * @brief at+creg?的上报
 * 
 * @param sStatus 网络信息的上报结构体
 * @param stat 网络状态,同sStatus.nStatus
 * @param reportN creg是否上报标志 正常情况为true
 * @param engine at引擎
 * @param ffw_cregreg creg set的值
 * @param nCurrRat 当前制式
 * @return uint8_t
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */

uint8_t ffw_cregrespond(ffw_nw_status_info_t *sStatus, uint8_t stat, bool reportN, void *engine, uint8_t ffw_cregreg, uint8_t nCurrRat);

/**
 * @brief creg主动上报
 * 
 * @param sStatus 网络信息的上报结构体
 * @param ffw_cregreg creg set的值
 * @param stat 网络状态,同sStatus.nStatus
 * @param PsStatus 网络状态
 * @return uint8_t
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */

void ffw_atcregrespond(ffw_nw_status_info_t *sStatus, uint8_t ffw_cregreg, uint8_t nCurrRat,uint8_t PsStatus );


/**
 * @brief cgreg主动上报
 * 
 * @param sStatus 网络信息的上报结构体
 * @param ffw_cgregreg cgreg set的值
 * @return uint8_t
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */
uint8_t ffw_atcgregrespond(ffw_nw_status_info_t *sStatus, uint8_t ffw_cgregreg);

/**
 * @brief at+cgreg?的上报
 * 
 * @param sStatus 网络信息的上报结构体
 * @param engine at引擎
 * @param ffw_cgregreg cgreg set的值
 * @param nCurrRat 当前制式
 * @return uint8_t
 * @retval FFW_R_SUCCESS: 成功
 * @retval FFW_R_FAILED: 失败
 */

uint8_t ffw_cgregrespond(ffw_nw_status_info_t *sStatus,void *engine, uint8_t ffw_cgregreg, uint8_t nCurrRat);


