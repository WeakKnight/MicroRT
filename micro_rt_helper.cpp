#include "micro_rt_helper.h"
#include "embree4/rtcore.h"
#include <tbb/tbb.h>

extern "C"
{
	void micro_rt_dispatch_embree_impl(micro_rt_tlas* as, micro_rt_ray* ray_list, micro_rt_hit* hit_list, uint32_t ray_num, uint8_t ray_mask, uint32_t hit_type)
	{
		const size_t block_size = 16;

		tbb::parallel_for(tbb::blocked_range<size_t>(0, ray_num, block_size),
			[block_size, as, ray_list, hit_list, ray_num, ray_mask, hit_type](const tbb::blocked_range<size_t>& range) {
				int validity[16];
				memset(validity, 0, sizeof(validity));

				RTCRayHit16 ray_packet;

				for (size_t i = range.begin(); i < range.end(); ++i)
				{
					micro_rt_ray& ray = ray_list[i];

					size_t local_index = i % block_size;

					ray_packet.ray.org_x[local_index] = ray.origin_x;
					ray_packet.ray.org_y[local_index] = ray.origin_y;
					ray_packet.ray.org_z[local_index] = ray.origin_z;

					ray_packet.ray.dir_x[local_index] = ray.dir_x;
					ray_packet.ray.dir_y[local_index] = ray.dir_y;
					ray_packet.ray.dir_z[local_index] = ray.dir_z;

					ray_packet.ray.tnear[local_index] = ray.tmin;
					ray_packet.ray.tfar[local_index] = ray.tmax;

					ray_packet.ray.time[local_index] = 0.0f;

					ray_packet.ray.flags[local_index] = 0;

					ray_packet.ray.mask[local_index] = ray_mask;

					ray_packet.hit.geomID[local_index] = RTC_INVALID_GEOMETRY_ID;
					ray_packet.hit.instID[0][local_index] = RTC_INVALID_GEOMETRY_ID;

					validity[local_index] = -1;
				}

				if (hit_type == MICRO_RT_HIT_TYPE_CLOSET)
				{
					rtcIntersect16(validity, (RTCScene)(as->api_handle), &ray_packet);
				}
				else if (hit_type == MICRO_RT_HIT_TYPE_ANY)
				{
					rtcOccluded16(validity, (RTCScene)(as->api_handle), &ray_packet.ray);
				}

				for (size_t i = range.begin(); i < range.end(); ++i)
				{
					micro_rt_hit& hit = hit_list[i];
					size_t local_index = i % block_size;

					if (hit_type == MICRO_RT_HIT_TYPE_CLOSET)
					{
						RTCHit16& embree_ray_hit = ray_packet.hit;

						if (embree_ray_hit.geomID[local_index] != RTC_INVALID_GEOMETRY_ID)
						{
							hit.instance_index = embree_ray_hit.instID[0][local_index];
							hit.geometry_index = embree_ray_hit.geomID[local_index];
							hit.primitive_index = embree_ray_hit.primID[local_index];
							hit.bary_x = embree_ray_hit.u[local_index];
							hit.bary_y = embree_ray_hit.v[local_index];
							hit.geometry_normal_x = embree_ray_hit.Ng_x[local_index];
							hit.geometry_normal_y = embree_ray_hit.Ng_y[local_index];
							hit.geometry_normal_z = embree_ray_hit.Ng_z[local_index];
						}
						else
						{
							hit.instance_index = ~0u;
						}
					}
					else if (hit_type == MICRO_RT_HIT_TYPE_ANY)
					{
						RTCRay16& embree_ray = ray_packet.ray;
						bool is_occluded = embree_ray.tfar[local_index] == -std::numeric_limits<float>::infinity() ? true : false;
						if (is_occluded)
						{
							hit.instance_index = 1u;
						}
						else
						{
							hit.instance_index = ~0u;
						}
					}
				}
			});
	}
}
