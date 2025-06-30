#ifndef CCB_INCLUDE_HK_ATCMD
#define CCB_INCLUDE_HK_ATCMD

#include <stdint.h>
#include <stdbool.h>

int32_t cbb_gtcellinfo(int32_t cmd_type, void *params, int32_t params_count, void *engine);
int32_t cbb_gtcellscan(int32_t cmd_type, void *params, int32_t params_count, void *engine);
#endif /* CCB_INCLUDE_CCB_HEARTBEAT */
