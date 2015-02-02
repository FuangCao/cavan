#pragma once

/*
 * File:		jwp_mcu.h
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

#ifdef _WIN32
#include "jwp-win32.h"
#else
#include <cavan/jwp-linux.h>
#endif

#define JWP_MCU_MTU					0xFF

#define JWP_MCU_MAGIC_SIZE			2
#define JWP_MCU_MAGIC_HIGH			0x12
#define JWP_MCU_MAGIC_LOW			0x34
#define JWP_MCU_MAGIC				(JWP_MCU_MAGIC_HIGH << 8 | JWP_MCU_MAGIC_LOW)
#define JWP_MCU_HEADER_SIZE			sizeof(struct jwp_mcu_header)
#define JWP_MCU_MAX_PAYLOAD			(JWP_MCU_MTU - JWP_MCU_HEADER_SIZE)
#define JWP_MCU_GET_PAYLOAD(hdr)	(((jwp_u8 *) hdr) + JWP_HEADER_SIZE)

typedef enum
{
    /* Application initial state */
    app_state_init = 0,

    /* Application is performing fast undirected advertisements */
    app_state_fast_advertising,

    /* Application is performing slow undirected advertisements */
    app_state_slow_advertising,

    /* Application is performing directed advertisements */
    app_state_directed_advertising,

    /* Connection has been established with the host */
    app_state_connected,

    /* Disconnection initiated by the application */
    app_state_disconnecting,

    /* Application is neither advertising nor connected to a host */
    app_state_idle

} app_state;

typedef enum
{
	JWP_CSR_CMD_SET_STATE,
	JWP_CSR_CMD_SET_FACTORY_SCAN,
	JWP_CSR_CMD_SET_WHITE_LIST,
	JWP_CSR_CMD_SET_WHITE_LIST_ENABLE,
	JWP_CSR_CMD_GET_STATE,
	JWP_CSR_CMD_GET_FIRMWARE_INFO,
	JWP_CSR_CMD_RM_PAIR,
	JWP_CSR_RESPONSE,
	JWP_CSR_EVENT_STATE,
	JWP_CSR_EVENT_FIRMWARE_INFO,
} jwp_csr_command_t;

typedef enum
{
	/* command */
	MCU_REQ_IDENTIFY = 1,
	MCU_REQ_ADD_OWNER = 2,
	MCU_REQ_REMOVE_OWNER = 3,
	MCU_REQ_OWNER_IN = 4,
	MCU_REQ_OWNER_OUT = 5,
	MCU_REQ_OWNER_LIST = 6,
	MCU_REQ_BATT_INFO = 7,
	MCU_REQ_GSM_INFO = 8,
	MCU_REQ_GPS_INFO = 9,
	MCU_REQ_RIDE_INFO = 10,
	MCU_REQ_HBT_INFO = 11,
	MCU_REQ_ALARM_INFO = 12,
	MCU_REQ_ALARM_SET = 13,
	MCU_REQ_SOS_INFO = 14,
	MCU_REQ_SOS_SET = 15,
	MCU_REQ_LIGHT_INFO = 16,
	MCU_REQ_LIGHT_SET = 17,
	MCU_REQ_TARGET_INFO = 18,
	MCU_REQ_TARGET_SET = 19,
	MCU_REQ_PRACTICE_INFO = 20,
	MCU_REQ_PRACTICE_SET = 21,
	MCU_REQ_VIBRATE_INFO = 22,
	MCU_REQ_VIBRATE_SET = 23,
	MCU_REQ_CALL_IN = 24,
	MCU_REQ_TIME_SET = 25,
	MCU_REQ_WEATHER_SET = 26,
	MCU_REQ_PHONE_BATT = 27,
	MCU_REQ_TRACK_INFO = 28,
	MCU_REQ_TRACK_SET = 29,
	MCU_REQ_GSM_SET = 30,
	MCU_REQ_GPS_SET = 31,
	MCU_REQ_SIM_INFO = 32,
	MCU_REQ_KEYLOCK_INFO = 33,
	MCU_REQ_KEYLOCK_SET = 34,
	MCU_REQ_NAVI_NOTIFY = 35,
	MCU_REQ_BAD_RIDE = 36,
	MCU_REQ_FILE_SEND = 37,
	MCU_REQ_READ_RECENT_RECORD = 38,
	/* response */
	MCU_RSP_OK = 200,
	MCU_RSP_ERROR = 201,
	MCU_RSP_IDENTIFY = 101,
	MCU_RSP_OWNER_IN = 104,
	MCU_RSP_OWNER_LIST = 106,
	MCU_RSP_BATT_INFO = 107,
	MCU_RSP_GSM_INFO = 108,
	MCU_RSP_GPS_INFO = 109,
	MCU_RSP_RIDE_INFO = 110,
	MCU_RSP_HBT_INFO = 111,
	MCU_RSP_ALARM_INFO = 112,
	MCU_RSP_SOS_INFO = 114,
	MCU_RSP_LIGHT_INFO = 116,
	MCU_RSP_TARGET_INFO = 118,
	MCU_RSP_PRACTICE_INFO = 120,
	MCU_RSP_VIBRATE_INFO = 122,
	MCU_RSP_TRACK_INFO = 128,
	MCU_RSP_SIM_INFO = 132,
	MCU_RSP_KEYLOCK_INFO = 133,
	MCU_RSP_READ_RECENT_RECORD = 138,
	/* event */
	MCU_EVT_BATT_INFO = 220,
	MCU_EVT_DEVICE_FAULT = 221,
	MCU_EVT_SOS_ACTIVITE = 222,
	MCU_EVT_RIDE_DATA = 223,
	MCU_EVT_TARGET_REACH = 224,
	MCU_EVT_HBT_DATA = 225,
} jwp_mcu_type_t;

typedef enum
{
	MCU_ERROR_INVALID = 1,		/* 指令不支持 */
	MCU_ERROR_FORMAT_FAULT,		/* 指令格式错误 */
	MCU_ERROR_NO_ARG,			/* 指令缺乏必要参数 */
	MCU_ERROR_NO_PERMISSION,	/* 执行权限不足 */
	MCU_ERROR_TIMEOUT,			/* 操作超时 */
	MCU_ERROR_OWNER_FULL,		/* 主人数已满 */
	MCU_ERROR_NO_NUMBER,		/* 无此号码 */
} jwp_mcu_error_t;

#pragma pack(1)
struct jwp_mcu_header
{
	union
	{
		jwp_u16 magic;
		struct
		{
			jwp_u8 magic_low;
			jwp_u8 magic_high;
		};
	};

	jwp_u8 type;
	jwp_u8 length;

#ifndef _WIN32
	jwp_u8 payload[0];
#endif
};

struct jwp_mcu_package
{
	union
	{
		struct
		{
			struct jwp_mcu_header header;
			jwp_u8 payload[JWP_MCU_MAX_PAYLOAD];
		};

		jwp_u8 body[JWP_MCU_MTU];
	};
};

struct jwp_mcu_response_identify
{
	jwp_u8 product_model;
	jwp_u8 firmware_version;
	jwp_u32 firmware_date;
	jwp_u32 device_sn;
};

struct jwp_mcu_request_add_owner
{
	jwp_u32 security_code;
	jwp_u32 owner;
};

struct jwp_mcu_request_remove_owner
{
	jwp_u32 owner;
};

struct jwp_mcu_request_owner_login
{
	jwp_u32 owner;
	jwp_u32 security_code;
};

struct jwp_mcu_response_owner_login
{
	jwp_u32 device_sn;
};

struct jwp_mcu_response_owner_list
{
	jwp_u32 owner1;
	jwp_u32 owner2;
	jwp_u32 owner3;
	jwp_u32 owner4;
	jwp_u32 owner5;
};

struct jwp_mcu_response_battery_info
{
	jwp_u8 state;
	jwp_u8 capacity_level;
	jwp_u8 capacity_percent;
};

struct jwp_mcu_response_gsm_info
{
	jwp_u8 register_state;
	jwp_u8 signal_level;
	jwp_u8 conn_state;
};

struct jwp_mcu_response_gps_info
{
	jwp_u8 state;
	jwp_u8 mode;
};

struct jwp_mcu_response_ride_info
{
	jwp_u8 speed;
	jwp_u8 speed_max;
	jwp_u8 speed_avg;
	jwp_u32 time;
	jwp_u32 mileage;
	jwp_u32 mileage_total;
	jwp_u32 time_total;
};

struct jwp_mcu_response_hbt_info
{
	jwp_u8 heart_rate;
};

struct jwp_mcu_response_alarm_info
{
	jwp_u8 state;
	jwp_u8 mode;
};

struct jwp_mcu_request_alarm_set
{
	jwp_u8 state;
};

struct jwp_mcu_response_sos_info
{
	jwp_u8 trigger;
	jwp_u8 enable;
	jwp_u8 delay;
};

struct jwp_mcu_request_sos_set
{
	jwp_u8 enable;
	jwp_u8 delay;
};

struct jwp_mcu_response_light_info
{
	jwp_u8 enable;
};

struct jwp_mcu_request_light_set
{
	jwp_u8 enable;
};

struct jwp_mcu_response_target_info
{
	jwp_u32 mileage_target;
	jwp_u32 mileage_now;
	jwp_u32 time_target;
	jwp_u32 time_now;
};

struct jwp_mcu_request_target_set
{
	jwp_u32 mileage_target;
	jwp_u32 time_target;
};

struct jwp_mcu_response_practice_info
{
	jwp_u8 enable;
};

struct jwp_mcu_request_practice_set
{
	jwp_u8 enable;
};

struct jwp_mcu_response_vibrate_info
{
	jwp_u8 enable;
};

struct jwp_mcu_request_vibrate_set
{
	jwp_u8 enable;
};

struct jwp_mcu_request_time_set
{
	jwp_u32 time;
};

struct jwp_mcu_request_weather_set
{
	jwp_u8 type;
	jwp_u8 temperature_min;
	jwp_u8 temperature_max;
};

struct jwp_mcu_request_phone_battery_set
{
	jwp_u8 charge_state;
	jwp_u8 capacity_level;
};

struct jwp_mcu_response_track_info
{
	jwp_u8 enable;
};

struct jwp_mcu_request_track_set
{
	jwp_u8 enable;
};

struct jwp_mcu_request_gsm_set
{
	jwp_u8 enable;
	jwp_u8 conn_enable;
	jwp_u8 airplane_mode;
};

struct jwp_mcu_request_gps_set
{
	jwp_u8 enable;
};

struct jwp_mcu_response_sim_info
{
	jwp_u8 present;
	jwp_u8 service_state;
	jwp_u8 imsi[15];
};

struct jwp_mcu_response_keylock_info
{
	jwp_u8 enable;
	jwp_u8 password[6];
};

struct jwp_mcu_request_keylock_set
{
	jwp_u8 enable;
	jwp_u8 password[6];
};

struct jwp_mcu_request_navi_notify_set
{
	jwp_u8 direction;
	jwp_u32 distance;
};

struct jwp_mcu_request_bad_navi_notify_set
{
	jwp_u8 direction;
	jwp_u32 distance;
};

struct jwp_mcu_request_file_transfer
{
	jwp_u8 type;
	jwp_u8 index;
	jwp_u8 total;
	jwp_u8 length;

#ifndef _WIN32
	jwp_u8 data[0];
#endif
};

struct jwp_mcu_response_read_recent_record
{
	jwp_u32 time_start;
	jwp_u32 time_end;
	jwp_u32 mileage;
	jwp_u8 speed_max;
	jwp_u8 heart_rate_avg;
	jwp_u8 heart_rate_max;
};

struct jwp_mcu_event_battery_info
{
	jwp_u8 state;
	jwp_u8 capacity_level;
	jwp_u8 capacity_percent;
};

struct jwp_mcu_event_device_fault
{
	jwp_u8 code;
};

struct jwp_mcu_event_sos
{
	jwp_u8 state;
};

struct jwp_mcu_event_ride_data
{
	jwp_u8 speed;
	jwp_u8 speed_max;
	jwp_u8 speed_avg;
	jwp_u32 time;
	jwp_u32 mileage;
	jwp_u32 mileage_total;
	jwp_u32 time_total;
};

struct jwp_mcu_event_target_reach
{
	jwp_u8 type;
	jwp_u32 value;
};

struct jwp_mcu_event_hbt_data
{
	jwp_u8 heart_rate;
};

struct jwp_csr_header
{
	jwp_u8 type;
};

struct jwp_csr_command_set_state
{
	struct jwp_csr_header header;
	jwp_u8 state;
};

struct jwp_csr_command_set_white_list
{
	struct jwp_csr_header header;
	jwp_u8 addr_list[5][6];
};

struct jwp_csr_command_set_white_list_enable
{
	struct jwp_csr_header header;
	jwp_u8 enable;
};

struct jwp_csr_command_set_factory_scan
{
	struct jwp_csr_header header;
	jwp_u8 mac_addr[6];
};

struct jwp_csr_response_package
{
	struct jwp_csr_header header;
	jwp_u8 success;
};

struct jwp_csr_event_state
{
	struct jwp_csr_header header;
	jwp_u8 state;
	jwp_u8 bonded;
	jwp_u8 bonded_addr[6];
};

struct jwp_csr_event_firmware_info
{
	struct jwp_csr_header header;
	jwp_u8 firmware_version;
	jwp_u8 mac_addr[6];
};
#pragma pack()

struct jwp_mcu_desc
{
	struct jwp_desc *jwp;
	struct jwp_mcu_package rx_pkg;
	struct jwp_package_receiver receiver;
	app_state csr_state;
	jwp_u8 csr_bonded;

	void *private_data;

	void (*csr_state_changed)(struct jwp_mcu_desc *mcu, const struct jwp_csr_event_state *event);
};

jwp_bool jwp_mcu_init(struct jwp_mcu_desc *mcu, struct jwp_desc *jwp);
jwp_bool jwp_mcu_send_package(struct jwp_mcu_desc *mcu, jwp_u8 type, const void *buff, jwp_size_t size);

jwp_bool jwp_csr_set_state(struct jwp_mcu_desc *mcu, app_state state);
jwp_bool jwp_csr_set_white_list(struct jwp_mcu_desc *mcu, jwp_u8 white_list[5][6]);
jwp_bool jwp_csr_set_white_list_enable(struct jwp_mcu_desc *mcu, jwp_u8 enable);
jwp_bool jwp_csr_set_factory_scan(struct jwp_mcu_desc *mcu, jwp_u8 addr[6]);

static inline void jwp_mcu_set_private_data(struct jwp_mcu_desc *mcu, void *data)
{
	mcu->private_data = data;
}

static inline void *jwp_mcu_get_private_data(struct jwp_mcu_desc *mcu)
{
	return mcu->private_data;
}

static inline jwp_bool jwp_mcu_send_ok_package(struct jwp_mcu_desc *mcu)
{
	return jwp_mcu_send_package(mcu, MCU_RSP_ERROR, NULL, 0);
}

static inline jwp_bool jwp_mcu_send_error_package(struct jwp_mcu_desc *mcu, jwp_u8 code)
{
	return jwp_mcu_send_package(mcu, MCU_RSP_ERROR, &code, 1);
}

static inline jwp_bool jwp_csr_send_command(struct jwp_mcu_desc *mcu, const void *command, jwp_size_t size)
{
	return jwp_send_command(mcu->jwp, command, size);
}

static inline jwp_bool jwp_csr_send_empty_command(struct jwp_mcu_desc *mcu, jwp_u8 type)
{
	return jwp_csr_send_command(mcu, &type, 1);
}

static inline jwp_bool jwp_csr_remove_pair(struct jwp_mcu_desc *mcu)
{
	return jwp_csr_send_empty_command(mcu, JWP_CSR_CMD_RM_PAIR);
}

static inline jwp_bool jwp_csr_get_state(struct jwp_mcu_desc *mcu)
{
	return jwp_csr_send_empty_command(mcu, JWP_CSR_CMD_GET_STATE);
}

static inline jwp_bool jwp_csr_get_firmware_info(struct jwp_mcu_desc *mcu)
{
	return jwp_csr_send_empty_command(mcu, JWP_CSR_CMD_GET_FIRMWARE_INFO);
}

static inline jwp_bool jwp_csr_start_advert(struct jwp_mcu_desc *mcu)
{
	return jwp_csr_set_state(mcu, app_state_fast_advertising);
}

static inline jwp_bool jwp_csr_disconnect(struct jwp_mcu_desc *mcu)
{
	return jwp_csr_set_state(mcu, app_state_disconnecting);
}

static inline jwp_bool jwp_csr_set_idle(struct jwp_mcu_desc *mcu)
{
	return jwp_csr_set_state(mcu, app_state_idle);
}
