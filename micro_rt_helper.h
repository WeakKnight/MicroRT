#pragma once
#include "micro_rt.h"

#ifdef __cplusplus
extern "C" {
#endif

void micro_rt_dispatch_embree_impl(micro_rt_tlas* as, micro_rt_ray* ray_list, micro_rt_hit* hit_list, uint32_t ray_num, uint8_t ray_mask, uint32_t hit_type);

#ifdef __cplusplus
}
#endif