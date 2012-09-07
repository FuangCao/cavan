/*
 * Author: Fuang.Cao
 * Email: cavan.cfa@gmail.com
 * Date: Thu Sep  6 09:34:48 CST 2012
 */

#include <cavan.h>
#include <cavan/sound.h>

#define USE_SND_PCM_SET_PARAMS	0

const char *wav_fmt_tosring(u16 fmt)
{
	switch (fmt)
	{
	case WAV_FMT_PCM:
		return "PCM";
	case WAV_FMT_DOLBY_AC3_SPDIF:
		return "DOLBY_AC3_SPDIF";
	case WAV_FMT_IEEE_FLOAT:
		return "IEEE_FLOAT";
	case WAV_FMT_EXTENSIBLE:
		return "EXTENSIBLE";
	}

	return "UNKNOWN";
}

snd_pcm_format_t wav_get_pcm_format(struct wav_format_chunk *fmt_chk)
{
	if (fmt_chk->format != WAV_FMT_PCM)
	{
		pr_red_info("Format is not PCM");
		return SND_PCM_FORMAT_UNKNOWN;
	}

	switch (fmt_chk->bits_per_sample)
	{
	case 16:
		pr_bold_info("SND_PCM_FORMAT_S16_LE");
		return SND_PCM_FORMAT_S16_LE;
	case 8:
		pr_bold_info("SND_PCM_FORMAT_U8");
		return SND_PCM_FORMAT_U8;
	default:
		pr_bold_info("SND_PCM_FORMAT_UNKNOWN");
		return SND_PCM_FORMAT_UNKNOWN;
	}
}

void show_wav_riff_chunk(struct wav_riff_chunk *chk)
{
	print_sep(60);
	pr_bold_info("wav_riff_chunk:");
	println("ID = %s", text_header((char *)&chk->id, sizeof(chk->id)));
	println("Type = %s", text_header((char *)&chk->type, sizeof(chk->type)));
	println("File Size = %s", size2text(chk->file_length));
	print_sep(60);
}

void show_wav_format_chunk(struct wav_format_chunk *chk)
{
	print_sep(60);
	pr_bold_info("wav_format_chunk:");
	println("ID = %s", text_header((char *)&chk->id, 3));
	println("Size = %d", chk->size);
	println("Format = %s", wav_fmt_tosring(chk->format));
	println("Channel Count = %d", chk->channels);
	println("Sample Rate = %d", chk->sample_rate);
	println("Bytes Per Second = %d", chk->bytes_per_sec);
	println("Block Align = %d", chk->block_align);
	println("Bits Per Sample = %d", chk->bits_per_sample);
	println("Addition = 0x%04x", chk->addition);
	print_sep(60);
}

void show_wav_fact_chunk(struct wav_fact_chunk *chk)
{
	print_sep(60);
	pr_bold_info("wav_fact_chunk:");
	println("ID = %s", text_header((char *)&chk->id, sizeof(chk->id)));
	println("Size = 0x%04x", chk->size);
	println("Data = 0x%04x", chk->data);
	print_sep(60);
}

void show_wav_file_header(struct wav_file_header *hdr)
{
	pr_bold_info("wav_file_header:");
	show_wav_riff_chunk(&hdr->riff);
	show_wav_format_chunk(&hdr->format);
}

void show_wav_data_chunk(struct wav_data_chunk *chk)
{
	print_sep(60);
	pr_bold_info("wav_data_chunk:");
	println("ID = %s", text_header((char *)&chk->id, sizeof(chk->id)));
	println("data_length = %d", chk->data_length);
	print_sep(60);
}

ssize_t wav_read_riff_chunk(int fd, struct wav_riff_chunk *riff_chk)
{
	ssize_t readlen;

	readlen = ffile_read(fd, riff_chk, sizeof(*riff_chk));
	if (readlen < 0)
	{
		return readlen;
	}

	show_wav_riff_chunk(riff_chk);

	if (wav_riff_check(riff_chk))
	{
		return readlen;
	}

	pr_red_info("wav_riff_check failed");

	ERROR_RETURN(EINVAL);
}

ssize_t wav_read_format_chunk(int fd, struct wav_format_chunk *fmt_chk)
{
	ssize_t readlen;

	readlen = ffile_read(fd, fmt_chk, sizeof(*fmt_chk) - sizeof(fmt_chk->addition));
	if (readlen < 0)
	{
		return readlen;
	}

	show_wav_format_chunk(fmt_chk);

	if (wav_format_check(fmt_chk) == false)
	{
		pr_red_info("wav_format_check failed");
		ERROR_RETURN(EINVAL);
	}

	if (fmt_chk->size == sizeof(*fmt_chk))
	{
		ssize_t readlen2;
		readlen2 = ffile_read(fd, &fmt_chk->addition, sizeof(fmt_chk->addition));
		if (readlen2 < 0)
		{
			return readlen2;
		}

		readlen += readlen2;
	}
	else
	{
		fmt_chk->addition = 0;
	}

	return readlen;
}

ssize_t wav_read_fact_chunk(int fd, struct wav_fact_chunk *fact_chk)
{
	ssize_t readlen;

	readlen = ffile_read(fd, fact_chk, sizeof(*fact_chk));
	if (readlen < 0)
	{
		return readlen;
	}

	show_wav_fact_chunk(fact_chk);

	if (wav_fact_check(fact_chk))
	{
		return readlen;
	}

	pr_red_info("wav_fact_check failed");

	ERROR_RETURN(EINVAL);
}

ssize_t wav_read_data_chunk(int fd, struct wav_data_chunk *data_chk)
{
	ssize_t hdrlen;

	hdrlen = ffile_read(fd, data_chk, sizeof(*data_chk));
	if (hdrlen < 0)
	{
		return hdrlen;
	}

	show_wav_data_chunk(data_chk);

	if (wav_data_check(data_chk) == false)
	{
		pr_red_info("wav_data_check failed");
		ERROR_RETURN(EINVAL);
	}

	return hdrlen;
}

ssize_t wav_read_file_header(int fd, struct wav_file_header *hdr)
{
	ssize_t rifflen, fmtlen, datalen;

	rifflen = wav_read_riff_chunk(fd, &hdr->riff);
	if (rifflen < 0)
	{
		pr_red_info("wav_read_riff_chunk failed");
		return rifflen;
	}

	fmtlen = wav_read_format_chunk(fd, &hdr->format);
	if (fmtlen < 0)
	{
		pr_red_info("wav_read_format_chunk failed");
		return fmtlen;
	}

	datalen = wav_read_data_chunk(fd, &hdr->data);
	if (datalen < 0)
	{
		pr_red_info("wav_read_data_chunk");
		return datalen;
	}

	return rifflen + fmtlen + datalen;
}

int open_wav_file(const char *filename, struct wav_file_header *hdr, int flags)
{
	int fd;
	ssize_t rwsize;

	fd = open(filename, flags, 0777);
	if (fd < 0)
	{
		print_error("open file `%s' failed", filename);
		return fd;
	}

	if (flags & (O_WRONLY | O_RDWR))
	{
		rwsize = ffile_write(fd, hdr, sizeof(*hdr));
		if (rwsize < 0)
		{
			print_error("Write to file `%s' failed", filename);
			close(fd);
			return rwsize;
		}
	}
	else
	{
		rwsize = wav_read_file_header(fd, hdr);
		if (rwsize < 0)
		{
			close(fd);
			return rwsize;
		}
	}

	return fd;
}

int cavan_wav_player_set_hwparams(struct cavan_wav_player *player)
{
	unsigned int rrate;
	int err, dir;
	snd_pcm_t *handle = player->pcm_handle;
	struct wav_format_chunk *fmt_chk = &player->wav_hdr.format;
	snd_pcm_hw_params_t *params;

	snd_pcm_hw_params_alloca(&params);
	player->format = wav_get_pcm_format(fmt_chk);
	player->bytes_per_sample = snd_pcm_format_physical_width(player->format);
	player->bytes_per_frame = player->bytes_per_sample * fmt_chk->channels;

	/* choose all parameters */
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0)
	{
		pr_red_info("Broken configuration for playback: no configurations available: %s", snd_strerror(err));
		return err;
	}
	/* set hardware resampling */
	err = snd_pcm_hw_params_set_rate_resample(handle, params, 1);
	if (err < 0)
	{
		pr_red_info("Resampling setup failed for playback: %s", snd_strerror(err));
		return err;
	}
	/* set the interleaved read/write format */
	err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0)
	{
		pr_red_info("Access type not available for playback: %s", snd_strerror(err));
		return err;
	}
	/* set the sample format */
	err = snd_pcm_hw_params_set_format(handle, params, player->format);
	if (err < 0)
	{
		pr_red_info("Sample format not available for playback: %s", snd_strerror(err));
		return err;
	}
	/* set the count of channels */
	err = snd_pcm_hw_params_set_channels(handle, params, fmt_chk->channels);
	if (err < 0)
	{
		pr_red_info("Channels count (%i) not available for playbacks: %s", fmt_chk->channels, snd_strerror(err));
		return err;
	}
	/* set the stream rate */
	rrate = fmt_chk->sample_rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rrate, 0);
	if (err < 0)
	{
		pr_red_info("Rate %iHz not available for playback: %s", fmt_chk->sample_rate, snd_strerror(err));
		return err;
	}

	printf("rrate = %iHz, rate = %iHz\n", rrate, fmt_chk->sample_rate);
	if (rrate != fmt_chk->sample_rate)
	{
		pr_red_info("Rate doesn't match (requested %iHz, get %iHz)", fmt_chk->sample_rate, err);
		return -EINVAL;
	}
	/* write the parameters to device */
	err = snd_pcm_hw_params(handle, params);
	if (err < 0)
	{
		pr_red_info("Unable to set hw params for playback: %s", snd_strerror(err));
		return err;
	}

	err = snd_pcm_hw_params_get_period_size(params, &player->period_size, &dir);
	if (err < 0)
	{
		pr_red_info("Unable to get period size for playback: %s", snd_strerror(err));
		return err;
	}

	player->buff_size = player->period_size * player->bytes_per_frame;

	return 0;
}

int cavan_wav_player_init(const char *filename, struct cavan_wav_player *player)
{
	int ret;

	player->fd = open_wav_file(filename, &player->wav_hdr, O_RDONLY);
	if (player->fd < 0)
	{
		return player->fd;
	}

	ret = snd_output_stdio_attach(&player->pcm_log, stdout, 0);
	if (ret < 0)
	{
		pr_snd_error("snd_output_stdio_attach", ret);
		goto out_close_fd;
	}

	ret = snd_pcm_open(&player->pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (ret < 0)
	{
		pr_snd_error("snd_pcm_open", ret);
		goto out_output_close;
	}

	ret = cavan_wav_player_set_hwparams(player);
	if (ret < 0)
	{
		pr_snd_error("cavan_wav_player_set_hw_params", ret);
		goto out_close_pcm;
	}

	snd_pcm_dump(player->pcm_handle, player->pcm_log);

	return 0;

out_close_pcm:
	snd_pcm_close(player->pcm_handle);
out_output_close:
	snd_output_close(player->pcm_log);
out_close_fd:
	close(player->fd);

	return ret;
}

void cavan_wav_player_uninit(struct cavan_wav_player *player)
{
	snd_output_close(player->pcm_log);
	snd_pcm_drain(player->pcm_handle);
	snd_pcm_close(player->pcm_handle);
	close(player->fd);
}

int cavan_wav_pcm_write(snd_pcm_t *handle, u32 frame_size, const void *buff, size_t size)
{
	int wrcount, count;

	for (count = size / frame_size; count; count -= wrcount, buff += wrcount * frame_size)
	{
		wrcount = snd_pcm_writei(handle, buff, count);
		if (wrcount < 0)
		{
			print_error("snd_pcm_writei");
			return wrcount;
		}
	}

	return 0;
}

int cavan_wav_player_run(struct cavan_wav_player *player)
{
	int fd = player->fd;
	snd_pcm_t *handle = player->pcm_handle;
	size_t buff_size = player->buff_size;
	void *buff = alloca(buff_size);
	size_t total_length = player->wav_hdr.data.data_length;
	ssize_t rdlen;
	int wrcount;
	int ret;

	ret = snd_pcm_prepare(handle);
	if (ret < 0)
	{
		pr_snd_error("snd_pcm_prepare", ret);
		return ret;
	}

	while (total_length)
	{
		rdlen = ffile_read(fd, buff, total_length > buff_size ? buff_size : total_length);
		if (rdlen <= 0)
		{
			print_error("ffile_read");
			return rdlen;
		}

		wrcount = cavan_wav_pcm_write(handle, player->bytes_per_frame, buff, rdlen);
		if (wrcount < 0)
		{
			print_error("snd_pcm_writei");
			return wrcount;
		}

		total_length -= rdlen;
	}

	return 0;
}

int cavan_wav_playback(const char *filename)
{
	int ret;
	struct cavan_wav_player player;

	ret = cavan_wav_player_init(filename, &player);
	if (ret < 0)
	{
		pr_red_info("cavan_wav_player_init");
		return ret;
	}

	ret = cavan_wav_player_run(&player);

	cavan_wav_player_uninit(&player);

	return ret;
}

