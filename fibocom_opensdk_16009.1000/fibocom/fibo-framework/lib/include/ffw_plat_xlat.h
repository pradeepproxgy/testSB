#ifndef C26EFA92_42E5_4ACB_A0E9_15382F6CA4C6
#define C26EFA92_42E5_4ACB_A0E9_15382F6CA4C6

#include <stdint.h>
#include <stdio.h>

/**
* @brief xlat output
* 
* @param p 
* @param size 
* @param nSim 
* @param nCid 
* @return int 
 */
int ffw_xlat_clat_output(void *pdata, int size, uint8_t nSim, uint8_t nCid);


int ffw_xlat_plat_input(uint8_t *data, int size, int *xlat_size);


int ffw_rat_xlat_address(const uint8_t **ipv6local, const uint8_t **plat, const uint8_t **dns6_1, const uint8_t **dns6_2);


int ffw_xlat_plat_addr_resolve(int cid, void (*cb)(void *arg), void *arg);
#endif /* C26EFA92_42E5_4ACB_A0E9_15382F6CA4C6 */
