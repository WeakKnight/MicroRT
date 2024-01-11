#pragma once

#include <stdint.h>

#define MICRO_RT_BACKEND_EMBREE 0
#define MICRO_RT_BACKEND_METAL 1

#define MICRO_RT_HIT_TYPE_CLOSET 0
#define MICRO_RT_HIT_TYPE_ANY 1

typedef struct
{
	void* api_handle;
} micro_rt_blas;

typedef struct
{
	void* api_handle;
} micro_rt_tlas;

typedef struct
{
	uint32_t index;
	uint8_t mask;
	float* transformation;
	micro_rt_blas* blas;
} micro_rt_instance_desc;

typedef struct
{
	void* vertexBuffer;
	uint32_t vertexByteOffset;
	uint32_t vertexByteStride;

	void* indexBuffer;
	uint32_t indexByteOffset;
	uint32_t indexByteStide;

	uint32_t vertexCount;
	uint32_t indexCount;
} micro_rt_geometry_desc;

typedef struct
{
	float origin_x;
	float origin_y;
	float origin_z;
	float dir_x;
	float dir_y;
	float dir_z;
	float tmin;
	float tmax;
} micro_rt_ray;

typedef struct
{
	uint32_t instance_index;
	uint32_t geometry_index;
	uint32_t primitive_index;
	float bary_x;
	float bary_y;
	float geometry_normal_x;
	float geometry_normal_y;
	float geometry_normal_z;
} micro_rt_hit;

/*
BLAS API
*/
micro_rt_blas* micro_rt_blas_create(micro_rt_geometry_desc* geometry_desc_list, uint32_t geometry_num);
void micro_rt_blas_destroy(micro_rt_blas* blas);

/*
TLAS API
*/
micro_rt_tlas* micro_rt_tlas_create(micro_rt_instance_desc* instance_desc_list, uint32_t instance_num);
void micro_rt_tlas_destroy(micro_rt_tlas* tlas);
void micro_rt_tlas_update_instance_transformation(micro_rt_tlas* as, uint32_t instance_index, float* transformation);
void micro_rt_tlas_update_instance_mask(micro_rt_tlas* as, uint32_t instance_index, uint8_t mask);
void micro_rt_tlas_commit(micro_rt_tlas* as);

/*
Life Cycle
*/
void micro_rt_init(uint32_t backend_type);
void micro_rt_dispatch(micro_rt_tlas* as, micro_rt_ray* ray_list, micro_rt_hit* hit_list, uint32_t ray_num, uint8_t ray_mask, uint32_t hit_type);
void micro_rt_release();