#include "micro_rt.h"
#include "micro_rt_helper.h"

#include "embree4/rtcore.h"
#include <malloc.h>

RTCDevice global_rtc_device = NULL;
uint32_t global_backend_type;

void micro_rt_init(uint32_t backend_type)
{
	global_backend_type = backend_type;

	if (backend_type == MICRO_RT_BACKEND_EMBREE)
	{
		global_rtc_device = rtcNewDevice(NULL);
	}
}

micro_rt_blas* micro_rt_blas_create(micro_rt_geometry_desc* geometry_desc_list, uint32_t geometry_num)
{
	if (global_backend_type == MICRO_RT_BACKEND_EMBREE)
	{
		micro_rt_blas* result = malloc(sizeof(micro_rt_blas));
		if (result == NULL)
		{
			return NULL;
		}

		RTCGeometry geometries[64];

		for (uint32_t geom_index = 0; geom_index < geometry_num; geom_index++)
		{
			micro_rt_geometry_desc geometryDesc = geometry_desc_list[geom_index];
			RTCGeometry geo = rtcNewGeometry(global_rtc_device, RTC_GEOMETRY_TYPE_TRIANGLE);
			geometries[geom_index] = geo;

			rtcSetSharedGeometryBuffer(geo, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT3, geometryDesc.vertexBuffer, geometryDesc.vertexByteOffset, geometryDesc.vertexByteStride, geometryDesc.vertexCount);
			rtcSetSharedGeometryBuffer(geo, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT3, geometryDesc.indexBuffer, geometryDesc.indexByteOffset, geometryDesc.indexByteStide * 3, geometryDesc.indexCount / 3);

			rtcCommitGeometry(geo);
		}

		RTCScene blas = rtcNewScene(global_rtc_device);
		result->api_handle = blas;

		for (uint32_t geom_index = 0; geom_index < geometry_num; geom_index++)
		{
			rtcAttachGeometry(blas, geometries[geom_index]);
		}

		rtcCommitScene(blas);

		for (uint32_t geom_index = 0; geom_index < geometry_num; geom_index++)
		{
			rtcReleaseGeometry(geometries[geom_index]);
		}

		return result;
	}

	return NULL;
}

micro_rt_tlas* micro_rt_tlas_create(micro_rt_instance_desc* instance_desc_list, uint32_t instance_num)
{
	if (global_backend_type == MICRO_RT_BACKEND_EMBREE)
	{
		micro_rt_tlas* result = malloc(sizeof(micro_rt_tlas));
		if (result == NULL)
		{
			return NULL;
		}

		RTCScene tlas = rtcNewScene(global_rtc_device);
		result->api_handle = tlas;

		rtcSetSceneFlags(tlas, RTC_SCENE_FLAG_DYNAMIC | RTC_SCENE_FLAG_ROBUST);
		rtcSetSceneBuildQuality(tlas, RTC_BUILD_QUALITY_LOW);

		for (uint32_t i = 0; i < instance_num; i++)
		{
			micro_rt_instance_desc instance_desc = instance_desc_list[instance_num];
			RTCScene blas = (RTCScene)instance_desc.blas->api_handle;
			RTCGeometry instance = rtcNewGeometry(global_rtc_device, RTC_GEOMETRY_TYPE_INSTANCE);

			rtcSetGeometryInstancedScene(instance, blas);
			rtcSetGeometryTimeStepCount(instance, 1);
			rtcSetGeometryTransform(instance, 0, RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR, instance_desc.transformation);
			rtcSetGeometryMask(instance, instance_desc.mask);
			rtcCommitGeometry(instance);

			rtcAttachGeometryByID(tlas, instance, instance_desc.index);

			rtcReleaseGeometry(instance);
		};

		rtcCommitScene(tlas);

		return result;
	}

	return NULL;
}

void micro_rt_blas_destroy(micro_rt_blas* blas)
{
	if (blas == NULL)
	{
		return;
	}

	if (global_backend_type == MICRO_RT_BACKEND_EMBREE)
	{
		rtcReleaseScene(blas->api_handle);
	}

	free(blas);
}

void micro_rt_tlas_destroy(micro_rt_tlas* tlas)
{
	if (tlas == NULL)
	{
		return;
	}

	if (global_backend_type == MICRO_RT_BACKEND_EMBREE)
	{
		rtcReleaseScene(tlas->api_handle);
	}

	free(tlas);
}

void micro_rt_tlas_update_instance_transformation(micro_rt_tlas* as, uint32_t instance_index, float* transformation)
{
	if (global_backend_type == MICRO_RT_BACKEND_EMBREE)
	{
		RTCScene tlas = as->api_handle;
		RTCGeometry instance = rtcGetGeometry(tlas, instance_index);
		rtcSetGeometryTransform(instance, 0, RTC_FORMAT_FLOAT4X4_COLUMN_MAJOR, transformation);
		rtcCommitGeometry(instance);
	}
}

void micro_rt_tlas_update_instance_mask(micro_rt_tlas* as, uint32_t instance_index, uint8_t mask)
{
	if (global_backend_type == MICRO_RT_BACKEND_EMBREE)
	{
		RTCScene tlas = (RTCScene)as->api_handle;
		RTCGeometry instance = rtcGetGeometry(tlas, instance_index);
		rtcSetGeometryMask(instance, mask);
		rtcCommitGeometry(instance);
	}
}

void micro_rt_dispatch(micro_rt_tlas* as, micro_rt_ray* ray_list, micro_rt_hit* hit_list, uint32_t ray_num, uint8_t ray_mask, uint32_t hit_type)
{
	if (global_backend_type == MICRO_RT_BACKEND_EMBREE)
	{
		micro_rt_dispatch_embree_impl(as, ray_list, hit_list, ray_num, ray_mask, hit_type);
	}
}

void micro_rt_tlas_commit(micro_rt_tlas* as)
{
	if (global_backend_type == MICRO_RT_BACKEND_EMBREE)
	{
		RTCScene tlas = (RTCScene)as->api_handle;
		rtcCommitScene(tlas);
	}
}

void micro_rt_release()
{
	if (global_backend_type == MICRO_RT_BACKEND_EMBREE)
	{
		rtcReleaseDevice(global_rtc_device);
	}
}