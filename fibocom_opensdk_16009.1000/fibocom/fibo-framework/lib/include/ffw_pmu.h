#ifndef FFW_HAL_PMU
#define FFW_HAL_PMU

bool ffw_pmu_ctrl(uint32_t id, bool enabled, bool lp_enabled);
int ffw_pmu_setlevel(uint8_t pmu_type, uint32_t level);

#endif
