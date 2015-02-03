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

#include "stdafx.h"

#ifdef _WIN32
#include "jwp_mcu.h"
#else
#include <cavan.h>
#include <cavan/jwp_mcu.h>
#endif

#define JWP_MCU_DEBUG		1

static void jwp_mcu_proccess_package(struct jwp_package_receiver *receiver)
{
	jwp_u8 rsplen;
	struct jwp_mcu_package *pkg = (struct jwp_mcu_package *) receiver->body;
	struct jwp_mcu_header *hdr = &pkg->header;
	struct jwp_mcu_desc *mcu = (struct jwp_mcu_desc *) jwp_package_receiver_get_private_data(receiver);

	switch (hdr->type)
	{
	case MCU_REQ_IDENTIFY:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_IDENTIFY\n");
#endif
		{
			struct jwp_mcu_response_identify *rsp = (struct jwp_mcu_response_identify *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_IDENTIFY;

			rsp->product_model = 0;
			rsp->firmware_version = 0;
			rsp->firmware_date = 0;
			rsp->device_sn = 0;
		}
		break;

	case MCU_REQ_ADD_OWNER:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_ADD_OWNER\n");
#endif
		{
			struct jwp_mcu_request_add_owner *rsp = (struct jwp_mcu_request_add_owner *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("security_code = 0x%08x, owner = 0x%08x\n", rsp->security_code, rsp->owner);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_REMOVE_OWNER:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_REMOVE_OWNER\n");
#endif
		{
			struct jwp_mcu_request_remove_owner *rsp = (struct jwp_mcu_request_remove_owner *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("owner = 0x%08x\n", rsp->owner);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_OWNER_IN:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_OWNER_IN\n");
#endif
		{
			struct jwp_mcu_request_owner_login *req = (struct jwp_mcu_request_owner_login *) JWP_MCU_GET_PAYLOAD(hdr);
			struct jwp_mcu_response_owner_login *rsp = (struct jwp_mcu_response_owner_login *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("security_code = 0x%08x, owner = 0x%08x\n", req->security_code, req->owner);
#endif

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_OWNER_IN;

			rsp->device_sn = 0;
		}
		break;

	case MCU_REQ_OWNER_OUT:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_OWNER_OUT\n");
#endif
		{
			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_OWNER_LIST:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_OWNER_LIST\n");
#endif
		{
			struct jwp_mcu_response_owner_list *rsp = (struct jwp_mcu_response_owner_list *) JWP_MCU_GET_PAYLOAD(hdr);

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
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_BATT_INFO\n");
#endif
		{
			struct jwp_mcu_response_battery_info *rsp = (struct jwp_mcu_response_battery_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_BATT_INFO;

			rsp->state = 0;
			rsp->capacity_level = 0;
			rsp->capacity_percent = 0;
		}
		break;

	case MCU_REQ_GSM_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_GSM_INFO\n");
#endif
		{
			struct jwp_mcu_response_gsm_info *rsp = (struct jwp_mcu_response_gsm_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_GSM_INFO;

			rsp->register_state = 0;
			rsp->signal_level = 0;
			rsp->conn_state = 0;
		}
		break;

	case MCU_REQ_GPS_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_GPS_INFO\n");
#endif
		{
			struct jwp_mcu_response_gps_info *rsp = (struct jwp_mcu_response_gps_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_GPS_INFO;

			rsp->state = 0;
			rsp->mode = 0;
		}
		break;

	case MCU_REQ_RIDE_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_RIDE_INFO\n");
#endif
		{
			struct jwp_mcu_response_ride_info *rsp = (struct jwp_mcu_response_ride_info *) JWP_MCU_GET_PAYLOAD(hdr);

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
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_HBT_INFO\n");
#endif
		{
			struct jwp_mcu_response_hbt_info *rsp = (struct jwp_mcu_response_hbt_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_HBT_INFO;

			rsp->heart_rate = 0;
		}
		break;

	case MCU_REQ_ALARM_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_ALARM_INFO\n");
#endif
		{
			struct jwp_mcu_response_alarm_info *rsp = (struct jwp_mcu_response_alarm_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_ALARM_INFO;

			rsp->state = 0;
			rsp->mode = 0;
		}
		break;

	case MCU_REQ_ALARM_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_ALARM_SET\n");
#endif
		{
			struct jwp_mcu_request_alarm_set *req = (struct jwp_mcu_request_alarm_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("state = %d\n", req->state);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_SOS_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_SOS_INFO\n");
#endif
		{
			struct jwp_mcu_response_sos_info *rsp = (struct jwp_mcu_response_sos_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_SOS_INFO;

			rsp->trigger = 0;
			rsp->enable = 0;
			rsp->delay = 0;
		}
		break;

	case MCU_REQ_SOS_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_SOS_SET\n");
#endif
		{
			struct jwp_mcu_request_sos_set *req = (struct jwp_mcu_request_sos_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("enable = %d, delay = %d\n", req->enable, req->delay);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_LIGHT_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_LIGHT_INFO\n");
#endif
		{
			struct jwp_mcu_response_light_info *rsp = (struct jwp_mcu_response_light_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_LIGHT_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_LIGHT_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_LIGHT_SET\n");
#endif
		{
			struct jwp_mcu_request_light_set *req = (struct jwp_mcu_request_light_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("enable = %d\n", req->enable);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_TARGET_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_TARGET_INFO\n");
#endif
		{
			struct jwp_mcu_response_target_info *rsp = (struct jwp_mcu_response_target_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_TARGET_INFO;

			rsp->mileage_target = 0;
			rsp->mileage_now = 0;
			rsp->time_target = 0;
			rsp->time_now = 0;
		}
		break;

	case MCU_REQ_TARGET_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_TARGET_SET\n");
#endif
		{
			struct jwp_mcu_request_target_set *req = (struct jwp_mcu_request_target_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("mileage_target = %d, time_target = %d\n", req->mileage_target, req->time_target);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_PRACTICE_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_PRACTICE_INFO\n");
#endif
		{
			struct jwp_mcu_response_practice_info *rsp = (struct jwp_mcu_response_practice_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_PRACTICE_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_PRACTICE_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_PRACTICE_SET\n");
#endif
		{
			struct jwp_mcu_request_practice_set *req = (struct jwp_mcu_request_practice_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("enable = %d\n", req->enable);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_VIBRATE_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_VIBRATE_INFO\n");
#endif
		{
			struct jwp_mcu_response_vibrate_info *rsp = (struct jwp_mcu_response_vibrate_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_VIBRATE_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_VIBRATE_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_VIBRATE_SET\n");
#endif
		{
			struct jwp_mcu_request_vibrate_set *req = (struct jwp_mcu_request_vibrate_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("enable = %d\n", req->enable);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_CALL_IN:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_CALL_IN\n");
#endif
		{
			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_TIME_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_TIME_SET\n");
#endif
		{
			struct jwp_mcu_request_time_set *req = (struct jwp_mcu_request_time_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("time = 0x%08x\n", req->time);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_WEATHER_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_WEATHER_SET\n");
#endif
		{
			struct jwp_mcu_request_weather_set *req = (struct jwp_mcu_request_weather_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("type = %d, temperature_min = %d, temperature_max = %d\n", req->type, req->temperature_min, req->temperature_max);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_PHONE_BATT:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_PHONE_BATT\n");
#endif
		{
			struct jwp_mcu_request_phone_battery_set *req = (struct jwp_mcu_request_phone_battery_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("charge_state = %d, capacity_level = %d\n", req->charge_state, req->capacity_level);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_TRACK_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_TRACK_INFO\n");
#endif
		{
			struct jwp_mcu_response_track_info *rsp = (struct jwp_mcu_response_track_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_TRACK_INFO;

			rsp->enable = 0;
		}
		break;

	case MCU_REQ_TRACK_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_TRACK_SET\n");
#endif
		{
			struct jwp_mcu_request_track_set *req = (struct jwp_mcu_request_track_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("enable = %d\n", req->enable);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_GSM_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_GSM_SET\n");
#endif
		{
			struct jwp_mcu_request_gsm_set *req = (struct jwp_mcu_request_gsm_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("enabel = %d, conn_enable = %d, airplane_mode = %d\n", req->enable, req->conn_enable, req->airplane_mode);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_GPS_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_GPS_SET\n");
#endif
		{
			struct jwp_mcu_request_gps_set *req = (struct jwp_mcu_request_gps_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("enable = %d\n", req->enable);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_SIM_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_SIM_INFO\n");
#endif
		{
			struct jwp_mcu_response_sim_info *rsp = (struct jwp_mcu_response_sim_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_SIM_INFO;

			rsp->present = 0;
			rsp->service_state = 0;
			memset(rsp->imsi, '8', sizeof(rsp->imsi));
		}
		break;

	case MCU_REQ_KEYLOCK_INFO:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_KEYLOCK_INFO\n");
#endif
		{
			struct jwp_mcu_response_keylock_info *rsp = (struct jwp_mcu_response_keylock_info *) JWP_MCU_GET_PAYLOAD(hdr);

			rsplen = sizeof(*rsp);
			hdr->type = MCU_RSP_KEYLOCK_INFO;

			rsp->enable = 0;
			memset(rsp->password, '8', sizeof(rsp->password));
		}
		break;

	case MCU_REQ_KEYLOCK_SET:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_KEYLOCK_SET\n");
#endif
		{
			struct jwp_mcu_request_keylock_set *req = (struct jwp_mcu_request_keylock_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("enable = %d, password = %s\n", req->enable, req->password);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_NAVI_NOTIFY:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_NAVI_NOTIFY\n");
#endif
		{
			struct jwp_mcu_request_navi_notify_set *req = (struct jwp_mcu_request_navi_notify_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("direction = %d, distance = %d\n", req->direction, req->distance);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_BAD_RIDE:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_BAD_RIDE\n");
#endif
		{
			struct jwp_mcu_request_bad_navi_notify_set *req = (struct jwp_mcu_request_bad_navi_notify_set *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("direction = %d, distance = %d\n", req->direction, req->distance);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_FILE_SEND:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_FILE_SEND\n");
#endif
		{
			struct jwp_mcu_request_file_transfer *req = (struct jwp_mcu_request_file_transfer *) JWP_MCU_GET_PAYLOAD(hdr);

#if JWP_PRINTF_ENABLE
			jwp_printf("type = %d, index = %d, total = %d, length = %d\n", req->type, req->index, req->total, req->length);
#endif

			rsplen = 0;
			hdr->type = MCU_RSP_OK;
		}
		break;

	case MCU_REQ_READ_RECENT_RECORD:
#if JWP_PRINTF_ENABLE
		jwp_printf("MCU_REQ_READ_RECENT_RECORD\n");
#endif
		{
			struct jwp_mcu_response_read_recent_record *rsp = (struct jwp_mcu_response_read_recent_record *) JWP_MCU_GET_PAYLOAD(hdr);

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
#if JWP_PRINTF_ENABLE
		jwp_printf("Invalid request = %d\n", hdr->type);
#endif
		{
#if 0
			rsplen = 1;
			hdr->type = MCU_RSP_ERROR;
			JWP_MCU_GET_PAYLOAD(hdr)[0] = MCU_ERROR_INVALID;
#else
			return;
#endif
		}
		break;
	}

	hdr->length = rsplen;

	jwp_send_data_all(mcu->jwp, (jwp_u8 *) hdr, JWP_MCU_HEADER_SIZE + rsplen);
}

static void jwp_mcu_data_received(struct jwp_desc *jwp, const void *buff, jwp_size_t size)
{
	struct jwp_mcu_desc *mcu = (struct jwp_mcu_desc *) jwp_get_private_data(jwp);

#if JWP_PRINTF_ENABLE
	jwp_printf("mcu data received: size = %d", size);
	jwp_dump_mem((const jwp_u8 *) buff, size);
#endif

#if JWP_RX_DATA_QUEUE_ENABLE
	while (jwp_package_receiver_fill_by_queue(&mcu->receiver, jwp_get_queue(jwp, JWP_QUEUE_RX_DATA)));
#else
	jwp_package_receiver_fill(&mcu->receiver, (const jwp_u8 *) buff, size);
#endif
}

// ============================================================

static void jwp_mcu_command_received(struct jwp_desc *jwp, const void *command, jwp_size_t size)
{
	struct jwp_mcu_desc *mcu = (struct jwp_mcu_desc *) jwp_get_private_data(jwp);
	struct jwp_csr_header *hdr;

	if (size < sizeof(struct jwp_csr_header))
	{
		return;
	}

	hdr = (struct jwp_csr_header *) command;
#if JWP_PRINTF_ENABLE
	jwp_printf("csr event type = %d\n", hdr->type);
#endif
	switch (hdr->type)
	{
	case JWP_CSR_EVENT_STATE:
		{
			struct jwp_csr_event_state *event = (struct jwp_csr_event_state *) command;

			mcu->csr_state = (app_state) event->state;
			mcu->csr_bonded = event->bonded;
			mcu->csr_state_changed(mcu, event);
		}
		break;

	case JWP_CSR_RESPONSE:
		{
			struct jwp_csr_response_package *rsp = (struct jwp_csr_response_package *) command;

#if JWP_PRINTF_ENABLE
			jwp_printf("csr response %s", rsp->success > 0 ? "true" : "false");
#endif
		}
		break;

	default:
#if JWP_SHOW_ERROR
		jwp_printf("Invalid csr package %d\n", hdr->type);
#endif
		break;
	}
}

jwp_bool jwp_csr_set_state(struct jwp_mcu_desc *mcu, app_state state)
{
	struct jwp_csr_command_set_state command;

	command.header.type = JWP_CSR_CMD_SET_STATE;
	command.state = state;

	return jwp_csr_send_command(mcu, &command, sizeof(command));
}

jwp_bool jwp_csr_set_white_list(struct jwp_mcu_desc *mcu, jwp_u8 white_list[5][6])
{
	struct jwp_csr_command_set_white_list command;

	command.header.type = JWP_CSR_CMD_SET_WHITE_LIST;
	memcpy(command.addr_list, white_list, sizeof(command.addr_list));

	return jwp_csr_send_command(mcu, &command, sizeof(command));
}

jwp_bool jwp_csr_set_white_list_enable(struct jwp_mcu_desc *mcu, jwp_u8 enable)
{
	struct jwp_csr_command_set_white_list_enable command;

	command.header.type = JWP_CSR_CMD_SET_WHITE_LIST_ENABLE;
	command.enable = enable;

	return jwp_csr_send_command(mcu, &command, sizeof(command));
}

jwp_bool jwp_csr_set_factory_scan(struct jwp_mcu_desc *mcu, jwp_u8 addr[6])
{
	struct jwp_csr_command_set_factory_scan command;

	command.header.type = JWP_CSR_CMD_SET_FACTORY_SCAN;
	memcpy(command.mac_addr, addr, sizeof(command.mac_addr));

	return jwp_csr_send_command(mcu, &command, sizeof(command));
}

// ============================================================

static jwp_size_t jwp_mcu_package_get_payload_length(struct jwp_package_receiver *receiver)
{
	struct jwp_mcu_header *hdr = (struct jwp_mcu_header *) receiver->body;

	return hdr->length;
}

jwp_bool jwp_mcu_init(struct jwp_mcu_desc *mcu, struct jwp_desc *jwp)
{
	mcu->jwp = jwp;
	jwp_set_private_data(jwp, mcu);
	jwp->data_received = jwp_mcu_data_received;
	jwp->command_received = jwp_mcu_command_received;

	mcu->rx_pkg.header.magic_low = JWP_MCU_MAGIC_LOW;
	mcu->rx_pkg.header.magic_high = JWP_MCU_MAGIC_HIGH;
	mcu->receiver.get_payload_length = jwp_mcu_package_get_payload_length;
	mcu->receiver.process_package = jwp_mcu_proccess_package;
	jwp_package_receiver_init(&mcu->receiver, mcu->rx_pkg.body, JWP_MCU_MAGIC_SIZE, JWP_MCU_HEADER_SIZE);
	jwp_package_receiver_set_private_data(&mcu->receiver, mcu);

	return true;
}

jwp_bool jwp_mcu_send_package(struct jwp_mcu_desc *mcu, jwp_u8 type, const void *data, jwp_size_t size)
{
	struct jwp_mcu_header hdr;

	hdr.magic_low = JWP_MCU_MAGIC_LOW;
	hdr.magic_high = JWP_MCU_MAGIC_HIGH;
	hdr.type = type;
	hdr.length = (jwp_u8) size;

	if (!jwp_send_data_all(mcu->jwp, (jwp_u8 *) &hdr, sizeof(hdr)))
	{
		return false;
	}

	if (size > 0)
	{
		return jwp_send_data_all(mcu->jwp, (jwp_u8 *) data, size);
	}

	return true;
}
