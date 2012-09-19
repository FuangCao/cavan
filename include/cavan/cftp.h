#pragma once

// Fuang.Cao <cavan.cfa@gmail.com> 2011-10-19 14:41:46

#include <cavan/network.h>
#include <cavan/usb.h>

#define CFTP_MIN_PACKAGE_LENGTH		KB(4)
#define CFTP_TIMEOUT_VALUE			5
#define CFTP_RETRY_COUNT			5

#pragma pack(1)
struct cftp_file_request
{
	u16 type;
	u32 size;
	u32 offset;
	u32 st_mode;
	u32 st_rdev;
	char filename[0];
};

struct cftp_data_package
{
	u16 type;
	u16 blk_num;
	u8 data[0];
};

struct cftp_ack_message
{
	u16 type;
	u16 blk_num;
};

struct cftp_error_message
{
	u16 type;
	u16 err_code;
	char message[0];
};

struct cftp_command_request
{
	u16 type;
	char command[0];
};

union cftp_message
{
	struct
	{
		u16 type;
		u8 data[CFTP_MIN_PACKAGE_LENGTH - 2];
	};
	struct cftp_file_request file_req;
	struct cftp_command_request cmd_req;
	struct cftp_data_package data_pkg;
	struct cftp_ack_message ack_msg;
	struct cftp_error_message err_msg;
};
#pragma pack()

enum cftp_package_type
{
	CFTP_PACKAGE_UNKNOWN,
	CFTP_PACKAGE_FILE_READ,
	CFTP_PACKAGE_FILE_WRITE,
	CFTP_PACKAGE_DATA,
	CFTP_PACKAGE_ACK,
	CFTP_PACKAGE_COMMAND,
	CFTP_PACKAGE_ERROR,
};

struct cftp_descriptor
{
	cavan_shared_data_t data;
	int fd;
	size_t max_xfer_length;
	u32 timeout_value;
	int retry_count;

	ssize_t (*send)(cavan_shared_data_t data, const void *buff, size_t size);
	ssize_t (*receive)(cavan_shared_data_t data, void *buff, size_t size);
	ssize_t (*send_timeout)(cavan_shared_data_t data, const void *buff, size_t size, u32 timeout);
	ssize_t (*receive_timeout)(cavan_shared_data_t data, void *buff, size_t size, u32 timeout);
	int (*receive_handle)(cavan_shared_data_t data, struct cftp_file_request *req);
	int (*send_handle)(cavan_shared_data_t data, struct cftp_file_request *req);
	int (*can_receive)(cavan_shared_data_t data, u32 timeout);
};

struct cftp_udp_link_descriptor
{
	int sockfd;
	struct sockaddr_in client_addr;
	socklen_t addrlen;
};

void cftp_descriptor_init(struct cftp_descriptor *desc);
int cftp_client_receive_file(struct cftp_descriptor *desc, const char *file_in, u32 offset_in, const char *file_out, u32 offset_out, size_t size);
int cftp_client_send_file(struct cftp_descriptor *desc, const char *file_in, u32 offset_in, const char *file_out, u32 offset_out, size_t size);
int cftp_server_receive_file(struct cftp_descriptor *desc, const char *filename, mode_t mode, u32 offset, size_t size);
int cftp_server_send_file(struct cftp_descriptor *desc, const char *filename, u32 offset, size_t size);
void *cftp_service_heandle(void *data);

ssize_t cftp_udp_receive_data(cavan_shared_data_t data, void *buff, size_t size);
ssize_t cftp_udp_send_data(cavan_shared_data_t data, const void *buff, size_t size);

ssize_t cftp_usb_receive_data(cavan_shared_data_t data, void *buff, size_t size);
ssize_t cftp_usb_send_data(cavan_shared_data_t data, const void *buff, size_t size);
ssize_t cftp_adb_receive_data(cavan_shared_data_t data, void *buff, size_t size);
ssize_t cftp_adb_send_data(cavan_shared_data_t data, const void *buff, size_t size);

