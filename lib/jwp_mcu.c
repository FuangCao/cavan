/*
 * File:		jwp_mcu.c
 * Author:		Fuang.Cao <cavan.cfa@gmail.com>
 * Created:		2015-01-31 14:05:28
 *
 * Copyright (c) 2015 Fuang.Cao <cavan.cfa@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <cavan.h>
#include <cavan/jwp_mcu.h>

#define JWP_MCU_DEBUG		1

static jwp_bool jwp_mcu_proccess_package(struct jwp_mcu_desc *mcu, struct jwp_mcu_header *hdr)
{
	// jwp_size_t rsplen;
	void *data = hdr->payload;

#if JWP_MCU_DEBUG
	jwp_printf("%s: type = %d\n", __FUNCTION__, hdr->type);
#endif

	switch (hdr->type)
	{
	case MCU_REQ_IDENTIFY_REQ:
		jwp_printf("MCU_REQ_IDENTIFY_REQ\n");
		{
			struct jwp_mcu_response_identify *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_IDENTIFY;

			rsp->product_model = 0;
			rsp->firmware_version = 0;
			rsp->firmware_date = 0;
			rsp->device_sn = 0;
		}
		break;

	case MCU_REQ_ADD_OWNER:
		jwp_printf("MCU_REQ_ADD_OWNER\n");
		{
			struct jwp_mcu_request_add_owner *rsp = data;

			jwp_printf("security_code = 0x%08x, owner = 0x%08x\n", rsp->security_code, rsp->owner);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_REMOVE_OWNER:
		jwp_printf("MCU_REQ_REMOVE_OWNER\n");
		{
			struct jwp_mcu_request_remove_owner *rsp = data;

			jwp_printf("owner = 0x%08x\n", rsp->owner);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_OWNER_IN:
		jwp_printf("MCU_REQ_OWNER_IN\n");
		{
			struct jwp_mcu_request_owner_login *req = data;
			struct jwp_mcu_response_owner_login *rsp = data;

			jwp_printf("security_code = 0x%08x, owner = 0x%08x\n", req->security_code, req->owner);

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_OWNER_IN;

			rsp->device_sn = 0;
		}
		break;

	case MCU_REQ_OWNER_OUT:
		jwp_printf("MCU_REQ_OWNER_OUT\n");
		{
			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_OWNER_LIST:
		jwp_printf("MCU_REQ_OWNER_LIST\n");
		{
			struct jwp_mcu_response_owner_list *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_OWNER_LIST;

			rsp->owner1 = 1;
			rsp->owner2 = 2;
			rsp->owner3 = 3;
			rsp->owner4 = 4;
			rsp->owner5 = 5;
		}
		break;

	case MCU_REQ_BATT_INFO:
		jwp_printf("MCU_REQ_BATT_INFO\n");
		{
			struct jwp_mcu_response_battery_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_BATT_INFO;

			rsp->state = 0;
			rsp->capacity_level = 0;
			rsp->capacity_percent = 0;
		}
		break;

	case MCU_REQ_GSM_INFO:
		jwp_printf("MCU_REQ_GSM_INFO\n");
		{
			struct jwp_mcu_response_gsm_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_GSM_INFO;

			rsp->register_state = 0;
			rsp->signal_level = 0;
			rsp->conn_state = 0;
		}
		break;

	case MCU_REQ_GPS_INFO:
		jwp_printf("MCU_REQ_GPS_INFO\n");
		{
			struct jwp_mcu_response_gps_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_GPS_INFO;

			rsp->state = 0;
			rsp->mode = 0;
		}
		break;

	case MCU_REQ_RIDE_INFO:
		jwp_printf("MCU_REQ_RIDE_INFO\n");
		{
			struct jwp_mcu_response_ride_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_RIDE_INFO;

			rsp->speed = 0;
			rsp->speed_max = 0;
			rsp->speed_avg = 0;
			rsp->time = 0;
			rsp->mileage = 0;
			rsp->mileage_total = 0;
			rsp->time_total = 0;
		}
		break;

	case MCU_REQ_HBT_INFO:
		jwp_printf("MCU_REQ_HBT_INFO\n");
		{
			struct jwp_mcu_response_hbt_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_HBT_INFO;

			rsp->heart_rate = 0;
		}
		break;

	case MCU_REQ_ALARM_INFO:
		jwp_printf("MCU_REQ_ALARM_INFO\n");
		{
			struct jwp_mcu_response_alarm_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_ALARM_INFO;

			rsp->state = 0;
			rsp->mode = 0;
		}
		break;

	case MCU_REQ_ALARM_SET:
		jwp_printf("MCU_REQ_ALARM_SET\n");
		{
			struct jwp_mcu_request_alarm_set *req = data;

			jwp_printf("state = %d\n", req->state);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_SOS_INFO:
		jwp_printf("MCU_REQ_SOS_INFO\n");
		{
			struct jwp_mcu_response_sos_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_SOS_INFO;

			rsp->trigger = 0;
			rsp->enable = 0;
			rsp->delay = 0;
		}
		break;

	case MCU_REQ_SOS_SET:
		jwp_printf("MCU_REQ_SOS_SET\n");
		{
			struct jwp_mcu_request_sos_set *req = data;

			jwp_printf("enable = %d, delay = %d\n", req->enable, req->delay);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_LIGHT_INFO:
		jwp_printf("MCU_REQ_LIGHT_INFO\n");
		{
			struct jwp_mcu_response_light_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_LIGHT_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_LIGHT_SET:
		jwp_printf("MCU_REQ_LIGHT_SET\n");
		{
			struct jwp_mcu_request_light_set *req = data;

			jwp_printf("enable = %d\n", req->enable);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_TARGET_INFO:
		jwp_printf("MCU_REQ_TARGET_INFO\n");
		{
			struct jwp_mcu_response_target_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_TARGET_INFO;

			rsp->mileage_target = 0;
			rsp->mileage_now = 0;
			rsp->time_target = 0;
			rsp->time_now = 0;
		}
		break;

	case MCU_REQ_TARGET_SET:
		jwp_printf("MCU_REQ_TARGET_SET\n");
		{
			struct jwp_mcu_request_target_set *req = data;

			jwp_printf("mileage_target = %d, time_target = %d\n", req->mileage_target, req->time_target);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_PRACTICE_INFO:
		jwp_printf("MCU_REQ_PRACTICE_INFO\n");
		{
			struct jwp_mcu_response_practice_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_PRACTICE_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_PRACTICE_SET:
		jwp_printf("MCU_REQ_PRACTICE_SET\n");
		{
			struct jwp_mcu_request_practice_set *req = data;

			jwp_printf("enable = %d\n", req->enable);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_VIBRATE_INFO:
		jwp_printf("MCU_REQ_VIBRATE_INFO\n");
		{
			struct jwp_mcu_response_vibrate_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_VIBRATE_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_VIBRATE_SET:
		jwp_printf("MCU_REQ_VIBRATE_SET\n");
		{
			struct jwp_mcu_request_vibrate_set *req = data;

			jwp_printf("enable = %d\n", req->enable);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_CALL_IN:
		jwp_printf("MCU_REQ_CALL_IN\n");
		{
			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_TIME_SET:
		jwp_printf("MCU_REQ_TIME_SET\n");
		{
			struct jwp_mcu_request_time_set *req = data;

			jwp_printf("time = 0x%08x\n", req->time);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_WEATHER_SET:
		jwp_printf("MCU_REQ_WEATHER_SET\n");
		{
			struct jwp_mcu_request_weather_set *req = data;

			jwp_printf("type = %d, temperature_min = %d, temperature_max = %d\n", req->type, req->temperature_min, req->temperature_max);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_PHONE_BATT:
		jwp_printf("MCU_REQ_PHONE_BATT\n");
		{
			struct jwp_mcu_request_phone_battery_set *req = data;

			jwp_printf("charge_state = %d, capacity_level = %d\n", req->charge_state, req->capacity_level);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_TRACK_INFO:
		jwp_printf("MCU_REQ_TRACK_INFO\n");
		{
			struct jwp_mcu_response_track_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_TRACK_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_TRACK_SET:
		jwp_printf("MCU_REQ_TRACK_SET\n");
		{
			struct jwp_mcu_request_track_set *req = data;

			jwp_printf("enable = %d\n", req->enable);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_GSM_SET:
		jwp_printf("MCU_REQ_GSM_SET\n");
		{
			struct jwp_mcu_request_gsm_set *req = data;

			jwp_printf("enabel = %d, conn_enable = %d, airplane_mode = %d\n", req->enable, req->conn_enable, req->airplane_mode);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_GPS_SET:
		jwp_printf("MCU_REQ_GPS_SET\n");
		{
			struct jwp_mcu_request_gps_set *req = data;

			jwp_printf("enable = %d\n", req->enable);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_SIM_INFO:
		jwp_printf("MCU_REQ_SIM_INFO\n");
		{
			struct jwp_mcu_response_sim_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_SIM_INFO;

			rsp->present = 0;
			rsp->service_state = 0;
			memset(rsp->imsi, '8', sizeof(rsp->imsi));
		}
		break;

	case MCU_REQ_KEYLOCK_INFO:
		jwp_printf("MCU_REQ_KEYLOCK_INFO\n");
		{
			struct jwp_mcu_response_keylock_info *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_KEYLOCK_INFO;

			rsp->enable = 0;
			memset(rsp->password, '8', sizeof(rsp->password));
		}
		break;

	case MCU_REQ_KEYLOCK_SET:
		jwp_printf("MCU_REQ_KEYLOCK_SET\n");
		{
			struct jwp_mcu_request_keylock_set *req = data;

			jwp_printf("enable = %d, password = %s\n", req->enable, req->password);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_NAVI_NOTIFY:
		jwp_printf("MCU_REQ_NAVI_NOTIFY\n");
		{
			struct jwp_mcu_request_navi_notify_set *req = data;

			jwp_printf("direction = %d, distance = %d\n", req->direction, req->distance);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_BAD_RIDE:
		jwp_printf("MCU_REQ_BAD_RIDE\n");
		{
			struct jwp_mcu_request_bad_navi_notify_set *req = data;

			jwp_printf("direction = %d, distance = %d\n", req->direction, req->distance);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_FILE_SEND:
		jwp_printf("MCU_REQ_FILE_SEND\n");
		{
			struct jwp_mcu_request_file_transfer *req = data;

			jwp_printf("type = %d, index = %d, total = %d, length = %d\n", req->type, req->index, req->total, req->length);

			// rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_READ_RECENT_RECORD:
		jwp_printf("MCU_REQ_READ_RECENT_RECORD\n");
		{
			struct jwp_mcu_response_read_recent_record *rsp = data;

			// rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_READ_RECENT_RECORD;

			rsp->time_start = 0;
			rsp->time_end = 0;
			rsp->mileage = 0;
			rsp->speed_max = 0;
			rsp->heart_rate_avg = 0;
			rsp->heart_rate_max = 0;
		}
		break;

	default:
		jwp_printf("Invalid request = %d\n", hdr->type);
		{
			// rsplen = 1;

#if 0
			hdr->type = MCU_RSP_ERROR;
			hdr->payload[0] = MCU_ERROR_INVALID;
#else
			return false;
#endif
		}
		break;
	}

	return jwp_send_data_all(mcu->jwp, (jwp_u8 *) hdr, JWP_MCU_MTU);
}

#if JWP_RX_DATA_QUEUE_ENABLE == 0
static void jwp_mcu_rx_package_init(struct jwp_mcu_rx_package *pkg)
{
	pkg->header.magic_low = JWP_MCU_MAGIC_LOW;
	pkg->header.magic_high = JWP_MCU_MAGIC_HIGH;

	pkg->head = pkg->body;
}

static jwp_size_t jwp_mcu_rx_package_fill(struct jwp_mcu_desc *mcu, const jwp_u8 *buff, jwp_size_t size)
{
	struct jwp_mcu_rx_package *pkg = &mcu->rx_pkg;

	if (pkg->head < pkg->body + 2)
	{
		if (pkg->head > pkg->body)
		{
			if (*buff == JWP_MCU_MAGIC_HIGH)
			{
				pkg->head = pkg->body + 2;
				pkg->remain = JWP_MCU_MTU - 2;
			}
			else
			{
				pkg->head = pkg->body;
			}
		}
		else if (buff[0] == JWP_MCU_MAGIC_LOW)
		{
			pkg->head = pkg->body + 1;
		}

		return 1;
	}

	if (size < pkg->remain)
	{
		jwp_memcpy(pkg->head, buff, size);

		pkg->head += size;
		pkg->remain -= size;
	}
	else
	{
		size = pkg->remain;
		jwp_memcpy(pkg->head, buff, size);

		jwp_mcu_proccess_package(mcu, &pkg->header);
		pkg->head = pkg->body;
	}

	return size;
}
#endif

static void jwp_mcu_data_received(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
#if JWP_RX_DATA_QUEUE_ENABLE
	struct jwp_queue *queue = jwp_get_queue(jwp, JWP_QUEUE_RX_DATA);

	while (jwp_queue_get_used_size(queue) >= sizeof(struct jwp_mcu_package))
	{
		struct jwp_mcu_package pkg;

		jwp_queue_dequeue_peek(queue, (jwp_u8 *) &pkg.header.magic, sizeof(pkg.header.magic));
		if (pkg.header.magic_low == JWP_MCU_MAGIC_LOW && pkg.header.magic_high == JWP_MCU_MAGIC_HIGH)
		{
			jwp_queue_dequeue(queue, (jwp_u8 *) &pkg, sizeof(pkg));
			jwp_mcu_proccess_package(&pkg.header);
		}
		else
		{
			jwp_queue_skip(queue, 1);
		}
	}
#else
	struct jwp_mcu_desc *mcu = jwp_get_private_data(jwp);

#if JWP_MCU_DEBUG
	jwp_printf("%s: size = %d\n", __FUNCTION__, size);
#endif

	while (1)
	{
		jwp_size_t wrlen;

		wrlen = jwp_mcu_rx_package_fill(mcu, buff, size);
		if (wrlen >= size)
		{
			break;
		}

		buff = (jwp_u8 *) buff + wrlen;
		size -= wrlen;
	}
#endif
}

jwp_bool jwp_mcu_init(struct jwp_mcu_desc *mcu, struct jwp_desc *jwp)
{
	mcu->jwp = jwp;
	jwp_set_private_data(jwp, mcu);
	jwp->data_received = jwp_mcu_data_received;

#if JWP_RX_DATA_QUEUE_ENABLE == 0
	jwp_mcu_rx_package_init(&mcu->rx_pkg);
#endif

	return true;
}

jwp_bool jwp_mcu_send_package(struct jwp_mcu_desc *mcu, jwp_u8 type, const void *data, jwp_size_t size)
{
	struct jwp_mcu_package pkg;
	struct jwp_mcu_header *hdr = &pkg.header;

	if (size > sizeof(pkg.payload))
	{
		return false;
	}

	hdr->magic_low = JWP_MCU_MAGIC_LOW;
	hdr->magic_high = JWP_MCU_MAGIC_HIGH;
	hdr->type = type;

	if (data && size)
	{
		memcpy(pkg.payload, data, size);
	}

	return jwp_send_data_all(mcu->jwp, (jwp_u8 *) &pkg, JWP_MCU_MTU);
}
