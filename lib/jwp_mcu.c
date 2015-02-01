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

static jwp_bool jwp_mcu_proccess_package(struct jwp_mcu_desc *mcu)
{
	jwp_size_t rsplen;
	struct jwp_mcu_rx_package *pkg = &mcu->rx_pkg;
	struct jwp_mcu_header *hdr = &pkg->header;

	pkg->head = pkg->body;

#if JWP_MCU_DEBUG
	jwp_printf("%s: type = %d, length = %d\n", __FUNCTION__, hdr->type, hdr->length);
#endif

	switch (hdr->type)
	{
	case MCU_REQ_IDENTIFY:
		jwp_printf("MCU_REQ_IDENTIFY\n");
		{
			struct jwp_mcu_response_identify *rsp = (struct jwp_mcu_response_identify *) hdr->payload;

			rsplen = sizeof(*rsp);
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
			struct jwp_mcu_request_add_owner *rsp = (struct jwp_mcu_request_add_owner *) hdr->payload;

			jwp_printf("security_code = 0x%08x, owner = 0x%08x\n", rsp->security_code, rsp->owner);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_REMOVE_OWNER:
		jwp_printf("MCU_REQ_REMOVE_OWNER\n");
		{
			struct jwp_mcu_request_remove_owner *rsp = (struct jwp_mcu_request_remove_owner *) hdr->payload;

			jwp_printf("owner = 0x%08x\n", rsp->owner);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_OWNER_IN:
		jwp_printf("MCU_REQ_OWNER_IN\n");
		{
			struct jwp_mcu_request_owner_login *req = (struct jwp_mcu_request_owner_login *) hdr->payload;
			struct jwp_mcu_response_owner_login *rsp = (struct jwp_mcu_response_owner_login *) hdr->payload;

			jwp_printf("security_code = 0x%08x, owner = 0x%08x\n", req->security_code, req->owner);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_OWNER_IN;

			rsp->device_sn = 0;
		}
		break;

	case MCU_REQ_OWNER_OUT:
		jwp_printf("MCU_REQ_OWNER_OUT\n");
		{
			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_OWNER_LIST:
		jwp_printf("MCU_REQ_OWNER_LIST\n");
		{
			struct jwp_mcu_response_owner_list *rsp = (struct jwp_mcu_response_owner_list *) hdr->payload;

			rsplen = sizeof(*rsp);
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
			struct jwp_mcu_response_battery_info *rsp = (struct jwp_mcu_response_battery_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_BATT_INFO;

			rsp->state = 0;
			rsp->capacity_level = 0;
			rsp->capacity_percent = 0;
		}
		break;

	case MCU_REQ_GSM_INFO:
		jwp_printf("MCU_REQ_GSM_INFO\n");
		{
			struct jwp_mcu_response_gsm_info *rsp = (struct jwp_mcu_response_gsm_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_GSM_INFO;

			rsp->register_state = 0;
			rsp->signal_level = 0;
			rsp->conn_state = 0;
		}
		break;

	case MCU_REQ_GPS_INFO:
		jwp_printf("MCU_REQ_GPS_INFO\n");
		{
			struct jwp_mcu_response_gps_info *rsp = (struct jwp_mcu_response_gps_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_GPS_INFO;

			rsp->state = 0;
			rsp->mode = 0;
		}
		break;

	case MCU_REQ_RIDE_INFO:
		jwp_printf("MCU_REQ_RIDE_INFO\n");
		{
			struct jwp_mcu_response_ride_info *rsp = (struct jwp_mcu_response_ride_info *) hdr->payload;

			rsplen = sizeof(*rsp);
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
			struct jwp_mcu_response_hbt_info *rsp = (struct jwp_mcu_response_hbt_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_HBT_INFO;

			rsp->heart_rate = 0;
		}
		break;

	case MCU_REQ_ALARM_INFO:
		jwp_printf("MCU_REQ_ALARM_INFO\n");
		{
			struct jwp_mcu_response_alarm_info *rsp = (struct jwp_mcu_response_alarm_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_ALARM_INFO;

			rsp->state = 0;
			rsp->mode = 0;
		}
		break;

	case MCU_REQ_ALARM_SET:
		jwp_printf("MCU_REQ_ALARM_SET\n");
		{
			struct jwp_mcu_request_alarm_set *req = (struct jwp_mcu_request_alarm_set *) hdr->payload;

			jwp_printf("state = %d\n", req->state);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_SOS_INFO:
		jwp_printf("MCU_REQ_SOS_INFO\n");
		{
			struct jwp_mcu_response_sos_info *rsp = (struct jwp_mcu_response_sos_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_SOS_INFO;

			rsp->trigger = 0;
			rsp->enable = 0;
			rsp->delay = 0;
		}
		break;

	case MCU_REQ_SOS_SET:
		jwp_printf("MCU_REQ_SOS_SET\n");
		{
			struct jwp_mcu_request_sos_set *req = (struct jwp_mcu_request_sos_set *) hdr->payload;

			jwp_printf("enable = %d, delay = %d\n", req->enable, req->delay);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_LIGHT_INFO:
		jwp_printf("MCU_REQ_LIGHT_INFO\n");
		{
			struct jwp_mcu_response_light_info *rsp = (struct jwp_mcu_response_light_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_LIGHT_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_LIGHT_SET:
		jwp_printf("MCU_REQ_LIGHT_SET\n");
		{
			struct jwp_mcu_request_light_set *req = (struct jwp_mcu_request_light_set *) hdr->payload;

			jwp_printf("enable = %d\n", req->enable);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_TARGET_INFO:
		jwp_printf("MCU_REQ_TARGET_INFO\n");
		{
			struct jwp_mcu_response_target_info *rsp = (struct jwp_mcu_response_target_info *) hdr->payload;

			rsplen = sizeof(*rsp);
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
			struct jwp_mcu_request_target_set *req = (struct jwp_mcu_request_target_set *) hdr->payload;

			jwp_printf("mileage_target = %d, time_target = %d\n", req->mileage_target, req->time_target);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_PRACTICE_INFO:
		jwp_printf("MCU_REQ_PRACTICE_INFO\n");
		{
			struct jwp_mcu_response_practice_info *rsp = (struct jwp_mcu_response_practice_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_PRACTICE_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_PRACTICE_SET:
		jwp_printf("MCU_REQ_PRACTICE_SET\n");
		{
			struct jwp_mcu_request_practice_set *req = (struct jwp_mcu_request_practice_set *) hdr->payload;

			jwp_printf("enable = %d\n", req->enable);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_VIBRATE_INFO:
		jwp_printf("MCU_REQ_VIBRATE_INFO\n");
		{
			struct jwp_mcu_response_vibrate_info *rsp = (struct jwp_mcu_response_vibrate_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_VIBRATE_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_VIBRATE_SET:
		jwp_printf("MCU_REQ_VIBRATE_SET\n");
		{
			struct jwp_mcu_request_vibrate_set *req = (struct jwp_mcu_request_vibrate_set *) hdr->payload;

			jwp_printf("enable = %d\n", req->enable);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_CALL_IN:
		jwp_printf("MCU_REQ_CALL_IN\n");
		{
			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_TIME_SET:
		jwp_printf("MCU_REQ_TIME_SET\n");
		{
			struct jwp_mcu_request_time_set *req = (struct jwp_mcu_request_time_set *) hdr->payload;

			jwp_printf("time = 0x%08x\n", req->time);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_WEATHER_SET:
		jwp_printf("MCU_REQ_WEATHER_SET\n");
		{
			struct jwp_mcu_request_weather_set *req = (struct jwp_mcu_request_weather_set *) hdr->payload;

			jwp_printf("type = %d, temperature_min = %d, temperature_max = %d\n", req->type, req->temperature_min, req->temperature_max);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_PHONE_BATT:
		jwp_printf("MCU_REQ_PHONE_BATT\n");
		{
			struct jwp_mcu_request_phone_battery_set *req = (struct jwp_mcu_request_phone_battery_set *) hdr->payload;

			jwp_printf("charge_state = %d, capacity_level = %d\n", req->charge_state, req->capacity_level);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_TRACK_INFO:
		jwp_printf("MCU_REQ_TRACK_INFO\n");
		{
			struct jwp_mcu_response_track_info *rsp = (struct jwp_mcu_response_track_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_TRACK_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_TRACK_SET:
		jwp_printf("MCU_REQ_TRACK_SET\n");
		{
			struct jwp_mcu_request_track_set *req = (struct jwp_mcu_request_track_set *) hdr->payload;

			jwp_printf("enable = %d\n", req->enable);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_GSM_SET:
		jwp_printf("MCU_REQ_GSM_SET\n");
		{
			struct jwp_mcu_request_gsm_set *req = (struct jwp_mcu_request_gsm_set *) hdr->payload;

			jwp_printf("enabel = %d, conn_enable = %d, airplane_mode = %d\n", req->enable, req->conn_enable, req->airplane_mode);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_GPS_SET:
		jwp_printf("MCU_REQ_GPS_SET\n");
		{
			struct jwp_mcu_request_gps_set *req = (struct jwp_mcu_request_gps_set *) hdr->payload;

			jwp_printf("enable = %d\n", req->enable);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_SIM_INFO:
		jwp_printf("MCU_REQ_SIM_INFO\n");
		{
			struct jwp_mcu_response_sim_info *rsp = (struct jwp_mcu_response_sim_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_SIM_INFO;

			rsp->present = 0;
			rsp->service_state = 0;
			memset(rsp->imsi, '8', sizeof(rsp->imsi));
		}
		break;

	case MCU_REQ_KEYLOCK_INFO:
		jwp_printf("MCU_REQ_KEYLOCK_INFO\n");
		{
			struct jwp_mcu_response_keylock_info *rsp = (struct jwp_mcu_response_keylock_info *) hdr->payload;

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_KEYLOCK_INFO;

			rsp->enable = 0;
			memset(rsp->password, '8', sizeof(rsp->password));
		}
		break;

	case MCU_REQ_KEYLOCK_SET:
		jwp_printf("MCU_REQ_KEYLOCK_SET\n");
		{
			struct jwp_mcu_request_keylock_set *req = (struct jwp_mcu_request_keylock_set *) hdr->payload;

			jwp_printf("enable = %d, password = %s\n", req->enable, req->password);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_NAVI_NOTIFY:
		jwp_printf("MCU_REQ_NAVI_NOTIFY\n");
		{
			struct jwp_mcu_request_navi_notify_set *req = (struct jwp_mcu_request_navi_notify_set *) hdr->payload;

			jwp_printf("direction = %d, distance = %d\n", req->direction, req->distance);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_BAD_RIDE:
		jwp_printf("MCU_REQ_BAD_RIDE\n");
		{
			struct jwp_mcu_request_bad_navi_notify_set *req = (struct jwp_mcu_request_bad_navi_notify_set *) hdr->payload;

			jwp_printf("direction = %d, distance = %d\n", req->direction, req->distance);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_FILE_SEND:
		jwp_printf("MCU_REQ_FILE_SEND\n");
		{
			struct jwp_mcu_request_file_transfer *req = (struct jwp_mcu_request_file_transfer *) hdr->payload;

			jwp_printf("type = %d, index = %d, total = %d, length = %d\n", req->type, req->index, req->total, req->length);

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_READ_RECENT_RECORD:
		jwp_printf("MCU_REQ_READ_RECENT_RECORD\n");
		{
			struct jwp_mcu_response_read_recent_record *rsp = (struct jwp_mcu_response_read_recent_record *) hdr->payload;

			rsplen = sizeof(*rsp);
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
#if 0
			rsplen = 1;
			hdr->type = MCU_RSP_ERROR;
			hdr->payload[0] = MCU_ERROR_INVALID;
#else
			return false;
#endif
		}
		break;
	}

	hdr->length = rsplen;

	return jwp_send_data_all(mcu->jwp, (jwp_u8 *) hdr, JWP_MCU_HEADER_SIZE + rsplen);
}

static void jwp_mcu_rx_package_init(struct jwp_mcu_rx_package *pkg)
{
	pkg->header.magic_low = JWP_MCU_MAGIC_LOW;
	pkg->header.magic_high = JWP_MCU_MAGIC_HIGH;

	pkg->head = pkg->body;
#if JWP_RX_DATA_QUEUE_ENABLE == 0
	pkg->header_start = pkg->body + JWP_MCU_MAGIC_SIZE;
#endif
	pkg->data_start = pkg->body + sizeof(pkg->header);
}

#if JWP_RX_DATA_QUEUE_ENABLE
static jwp_bool jwp_mcu_rx_package_fill(struct jwp_mcu_desc *mcu)
{
	jwp_size_t rdlen;
	jwp_size_t remain;
	struct jwp_mcu_rx_package *pkg = &mcu->rx_pkg;
	struct jwp_queue *queue = jwp_get_queue(mcu->jwp, JWP_QUEUE_RX_DATA);

	if (pkg->head < pkg->data_start)
	{
		if (pkg->head == pkg->body)
		{
			while (1)
			{
				rdlen = jwp_queue_dequeue_peek(queue, pkg->head, JWP_MCU_MAGIC_SIZE);
				if (rdlen < JWP_MCU_MAGIC_SIZE)
				{
					return false;
				}

				if (pkg->head[0] == JWP_MCU_MAGIC_LOW && pkg->head[1] == JWP_MCU_MAGIC_HIGH)
				{
					break;
				}

				jwp_queue_skip(queue, 1);
			}

			jwp_queue_dequeue_commit(queue);

			pkg->head = pkg->body + JWP_MCU_MAGIC_SIZE;
		}

		remain = pkg->data_start - pkg->head;
		rdlen = jwp_queue_dequeue(queue, pkg->head, remain);
		if (rdlen < remain)
		{
			pkg->head += rdlen;
			return false;
		}

		pkg->head = pkg->data_start;
		pkg->data_end = pkg->head + pkg->header.length;
	}

	remain = pkg->data_end - pkg->head;
	rdlen = jwp_queue_dequeue(queue, pkg->head, remain);
	if (rdlen < remain)
	{
		pkg->head += rdlen;
		return false;
	}

	jwp_mcu_proccess_package(mcu);

	return true;

}
#else
static jwp_size_t jwp_mcu_rx_package_fill(struct jwp_mcu_desc *mcu, const jwp_u8 *buff, jwp_size_t size)
{
	jwp_size_t remain;
	struct jwp_mcu_rx_package *pkg = &mcu->rx_pkg;

	if (pkg->head < pkg->data_start)
	{
		if (pkg->head < pkg->header_start)
		{
			if (pkg->head == pkg->body)
			{
				if (*buff == JWP_MCU_MAGIC_LOW)
				{
					pkg->head++;
				}
			}
			else if (*buff == JWP_MCU_MAGIC_HIGH)
			{
				pkg->head++;
			}
			else
			{
				pkg->head = pkg->body;
			}

			return 1;
		}

		remain = pkg->data_start - pkg->header_start;
		if (size < remain)
		{
			jwp_memcpy(pkg->head, buff, size);
			pkg->head += size;

			return size;
		}

		jwp_memcpy(pkg->head, buff, remain);

		if (pkg->header.length == 0)
		{
			goto label_process_package;
		}

		pkg->head = pkg->data_start;
		pkg->data_end = pkg->head + pkg->header.length;

		return remain;
	}

	remain = pkg->data_end - pkg->data_start;
	if (size < remain)
	{
		jwp_memcpy(pkg->head, buff, size);
		pkg->head += size;

		return size;
	}

	jwp_memcpy(pkg->head, buff, remain);

label_process_package:
	jwp_mcu_proccess_package(mcu);

	return remain;
}
#endif

static void jwp_mcu_data_received(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
	struct jwp_mcu_desc *mcu = jwp_get_private_data(jwp);
#if JWP_RX_DATA_QUEUE_ENABLE
	while (jwp_mcu_rx_package_fill(mcu));
#else
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

	jwp_mcu_rx_package_init(&mcu->rx_pkg);

	return true;
}

jwp_bool jwp_mcu_send_package(struct jwp_mcu_desc *mcu, jwp_u8 type, const void *data, jwp_size_t size)
{
	struct jwp_mcu_header hdr;

	hdr.magic_low = JWP_MCU_MAGIC_LOW;
	hdr.magic_high = JWP_MCU_MAGIC_HIGH;
	hdr.type = type;
	hdr.length = size;

	if (!jwp_send_data_all(mcu->jwp, (jwp_u8 *) &hdr, sizeof(hdr)))
	{
		return false;
	}

	if (size > 0)
	{
		return jwp_send_data_all(mcu->jwp, data, size);
	}

	return true;
}
