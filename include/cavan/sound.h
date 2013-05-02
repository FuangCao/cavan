#pragma once

/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Sep  6 09:34:48 CST 2012
 */

#include <cavan.h>
#include <alsa/asoundlib.h>

#define WAV_BUILD_ID(b1, b2, b3, b4) \
	BYTES_DWORD(b4, b3, b2, b1)

#define WAV_RIFF_ID				WAV_BUILD_ID('R', 'I', 'F', 'F')
#define WAV_WAVE_ID				WAV_BUILD_ID('W', 'A', 'V', 'E')
#define WAV_FACT_ID				WAV_BUILD_ID('f', 'a', 'c', 't')
#define WAV_DATA_ID				WAV_BUILD_ID('d', 'a', 't', 'a')
#define WAV_FMT_PCM				0x0001
#define WAV_FMT_IEEE_FLOAT		0x0003
#define WAV_FMT_DOLBY_AC3_SPDIF	0x0092
#define WAV_FMT_EXTENSIBLE		0xFFFE

#define WAV_BUFFER_TIME			500000
#define WAV_PERIOD_TIME			100000

#define pr_snd_error(fmt, err, args ...) \
	print_error(fmt ": %s", ##args, snd_strerror(err))

#pragma pack(1)
struct wav_riff_chunk
{
	u32 id;		// "RIFF"
	u32 file_length;
	u32 type;	// "WAVE"
};

struct wav_format_chunk
{
	u32 id;		// "fmt"
	u32 size;
	u16 format;
	u16 channels;
	u32 sample_rate;
	u32 bytes_per_sec;
	u16 block_align;
	u16 bits_per_sample;
	u16 addition;
};

struct wav_fact_chunk
{
	u32 id;		// "fact"
	u32 size;
	u32 data;
};

struct wav_data_chunk
{
	u32 id;		// "data"
	u32 data_length;
};

struct wav_file_header
{
	struct wav_riff_chunk riff;
	struct wav_format_chunk format;
	struct wav_data_chunk data;
};
#pragma pack()

struct cavan_wav_player
{
	int fd;
	u32 bytes_per_sample;
	u32 bytes_per_frame;
	snd_pcm_uframes_t buffer_size, period_size;
	snd_pcm_format_t format;
	struct wav_file_header wav_hdr;
	snd_output_t *pcm_log;
	snd_pcm_t *pcm_handle;
	size_t buff_size;
};

const char *wav_fmt_tosring(u16 fmt);
snd_pcm_format_t wav_get_pcm_format(struct wav_format_chunk *chk);
void show_wav_riff_chunk(struct wav_riff_chunk *chk);
void show_wav_format_chunk(struct wav_format_chunk *chk);
void show_wav_fact_chunk(struct wav_fact_chunk *chk);
void show_wav_file_header(struct wav_file_header *hdr);
void show_wav_data_chunk(struct wav_data_chunk *chk);
ssize_t wav_read_riff_chunk(int fd, struct wav_riff_chunk *riff_chk);
ssize_t wav_read_format_chunk(int fd, struct wav_format_chunk *fmt_chk);
ssize_t wav_read_fact_chunk(int fd, struct wav_fact_chunk *fact_chk);
ssize_t wav_read_data_chunk(int fd, struct wav_data_chunk *data_chk);
ssize_t wav_read_file_header(int fd, struct wav_file_header *hdr);
int open_wav_file(const char *filename, struct wav_file_header *hdr, int flags);
int cavan_wav_player_init(const char *filename, struct cavan_wav_player *player);
int cavan_wav_player_set_hwparams(struct cavan_wav_player *player);
int cavan_wav_player_set_swparams(struct cavan_wav_player *player, int period_event);
int cavan_wav_player_xrun_recovery(snd_pcm_t *handle, int err);
void cavan_wav_player_deinit(struct cavan_wav_player *player);
int cavan_wav_playback(const char *filename);

static inline int wav_riff_check(struct wav_riff_chunk *chk)
{
	return chk->id == WAV_RIFF_ID && chk->type == WAV_WAVE_ID;
}

static inline int wav_format_check(struct wav_format_chunk *chk)
{
	return text_lhcmp("fmt", (char *)&chk->id) == 0;
}

static inline int wav_fact_check(struct wav_fact_chunk *chk)
{
	return chk->id == WAV_FACT_ID;
}

static inline int wav_data_check(struct wav_data_chunk *chk)
{
	return chk->id == WAV_DATA_ID;
}
