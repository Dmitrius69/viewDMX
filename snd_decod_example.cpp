/*
 * (C) 2010-2013 Stefan Seyfried
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
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * cAudio implementation with decoder.
 * uses libao  <http://www.xiph.org/ao/> for output
 *      ffmpeg <http://ffmpeg.org> for demuxing / decoding / format conversion
 */

#include <cstdio>
#include <cstdlib>

#include "audio_lib.h"
#include "dmx_lib.h"
#include "lt_debug.h"

#define lt_debug(args...) _lt_debug(HAL_DEBUG_AUDIO, this, args)
#define lt_info(args...) _lt_info(HAL_DEBUG_AUDIO, this, args)


extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
#include <ao/ao.h>
}
/* ffmpeg buf 2k */
#define INBUF_SIZE 0x0800
/* my own buf 16k */
#define DMX_BUF_SZ 0x4000

cAudio * audioDecoder = NULL;
extern cDemux *audioDemux;
static uint8_t *dmxbuf = NULL;
static int bufpos;

extern bool HAL_nodec;

static cAudio *gThiz = NULL;

static ao_device *adevice = NULL;
static ao_sample_format sformat;

static AVCodecContext *c= NULL;

cAudio::cAudio(void *, void *, void *)
{
	thread_started = false;
	if (!HAL_nodec)
		dmxbuf = (uint8_t *)malloc(DMX_BUF_SZ);
	bufpos = 0;
	curr_pts = 0;
	gThiz = this;
	ao_initialize();
}

cAudio::~cAudio(void)
{
	closeDevice();
	free(dmxbuf);
	if (adevice)
		ao_close(adevice);
	adevice = NULL;
	ao_shutdown();
}

void cAudio::openDevice(void)
{
	lt_debug("%s\n", __func__);
}

void cAudio::closeDevice(void)
{
	lt_debug("%s\n", __func__);
}

int cAudio::do_mute(bool enable, bool remember)
{
	lt_debug("%s(%d, %d)\n", __func__, enable, remember);
	return 0;
}

int cAudio::setVolume(unsigned int left, unsigned int right)
{
	lt_debug("%s(%d, %d)\n", __func__, left, right);
	return 0;
}

int cAudio::Start(void)
{
	lt_debug("%s >\n", __func__);
	if (! HAL_nodec)
		Thread::startThread();
	lt_debug("%s <\n", __func__);
	return 0;
}

int cAudio::Stop(void)
{
	lt_debug("%s >\n", __func__);
	if (thread_started)
	{
		thread_started = false;
		Thread::joinThread();
	}
	lt_debug("%s <\n", __func__);
	return 0;
}

bool cAudio::Pause(bool /*Pcm*/)
{
	return true;
};

void cAudio::SetSyncMode(AVSYNC_TYPE Mode)
{
	lt_debug("%s %d\n", __func__, Mode);
};

void cAudio::SetStreamType(AUDIO_FORMAT type)
{
	lt_debug("%s %d\n", __func__, type);
};

int cAudio::setChannel(int /*channel*/)
{
	return 0;
};

int cAudio::PrepareClipPlay(int ch, int srate, int bits, int le)
{
	lt_debug("%s ch %d srate %d bits %d le %d adevice %p\n", __func__, ch, srate, bits, le, adevice);;
	int driver;
	int byte_format = le ? AO_FMT_LITTLE : AO_FMT_BIG;
	if (sformat.bits != bits || sformat.channels != ch || sformat.rate != srate ||
	    sformat.byte_format != byte_format || adevice == NULL)
	{
		driver = ao_default_driver_id();
		sformat.bits = bits;
		sformat.channels = ch;
		sformat.rate = srate;
		sformat.byte_format = byte_format;
		sformat.matrix = 0;
		if (adevice)
			ao_close(adevice);
		adevice = ao_open_live(driver, &sformat, NULL);
		ao_info *ai = ao_driver_info(driver);
		lt_info("%s: changed params ch %d srate %d bits %d le %d adevice %p\n",
			__func__, ch, srate, bits, le, adevice);;
		lt_info("libao driver: %d name '%s' short '%s' author '%s'\n",
				driver, ai->name, ai->short_name, ai->author);
	}
	return 0;
};

int cAudio::WriteClip(unsigned char *buffer, int size)
{
	lt_debug("cAudio::%s buf 0x%p size %d\n", __func__, buffer, size);
	if (!adevice) {
		lt_info("%s: adevice not opened?\n", __func__);
		return 0;
	}
	ao_play(adevice, (char *)buffer, size);
	return size;
};

int cAudio::StopClip()
{
	lt_debug("%s\n", __func__);
#if 0
	/* don't do anything - closing / reopening ao all the time makes for long delays
	 * reinit on-demand (e.g. for changed parameters) instead */
	if (!adevice) {
		lt_info("%s: adevice not opened?\n", __func__);
		return 0;
	}
	ao_close(adevice);
	adevice = NULL;
#endif
	return 0;
};

void cAudio::getAudioInfo(int &type, int &layer, int &freq, int &bitrate, int &mode)
{
	type = 0;
	layer = 0;	/* not used */
	freq = 0;
	bitrate = 0;	/* not used, but easy to get :-) */
	mode = 0;	/* default: stereo */
	if (c) {
		type = (c->codec_id != AV_CODEC_ID_MP2); /* only mpeg / not mpeg is indicated */
		freq = c->sample_rate;
		bitrate = c->bit_rate;
		if (c->channels == 1)
			mode = 3; /* for AV_CODEC_ID_MP2, only stereo / mono is detected for now */
		if (c->codec_id != AV_CODEC_ID_MP2) {
			switch (c->channel_layout) {
				case AV_CH_LAYOUT_MONO:
					mode = 1;	// "C"
					break;
				case AV_CH_LAYOUT_STEREO:
					mode = 2;	// "L/R"
					break;
				case AV_CH_LAYOUT_2_1:
				case AV_CH_LAYOUT_SURROUND:
					mode = 3;	// "L/C/R"
					break;
				case AV_CH_LAYOUT_2POINT1:
					mode = 4;	// "L/R/S"
					break;
				case AV_CH_LAYOUT_3POINT1:
					mode = 5;	// "L/C/R/S"
					break;
				case AV_CH_LAYOUT_2_2:
				case AV_CH_LAYOUT_QUAD:
					mode = 6;	// "L/R/SL/SR"
					break;
				case AV_CH_LAYOUT_5POINT0:
				case AV_CH_LAYOUT_5POINT1:
					mode = 7;	// "L/C/R/SL/SR"
					break;
				default:
					lt_info("%s: unknown ch_layout 0x%" PRIx64 "\n",
						 __func__, c->channel_layout);
			}
		}
	}
	lt_debug("%s t: %d l: %d f: %d b: %d m: %d codec_id: %x\n",
		  __func__, type, layer, freq, bitrate, mode, c->codec_id);
};

void cAudio::SetSRS(int /*iq_enable*/, int /*nmgr_enable*/, int /*iq_mode*/, int /*iq_level*/)
{
	lt_debug("%s\n", __func__);
};

void cAudio::SetHdmiDD(bool enable)
{
	lt_debug("%s %d\n", __func__, enable);
};

void cAudio::SetSpdifDD(bool enable)
{
	lt_debug("%s %d\n", __func__, enable);
};

void cAudio::ScheduleMute(bool On)
{
	lt_debug("%s %d\n", __func__, On);
};

void cAudio::EnableAnalogOut(bool enable)
{
	lt_debug("%s %d\n", __func__, enable);
};

void cAudio::setBypassMode(bool disable)
{
	lt_debug("%s %d\n", __func__, disable);
}

static int _my_read(void *, uint8_t *buf, int buf_size)
{
	return gThiz->my_read(buf, buf_size);
}

int cAudio::my_read(uint8_t *buf, int buf_size)
{
	int tmp = 0;
	if (audioDecoder && bufpos < DMX_BUF_SZ - 4096) {
		while (bufpos < buf_size && ++tmp < 20) { /* retry max 20 times */
			int ret = audioDemux->Read(dmxbuf + bufpos, DMX_BUF_SZ - bufpos, 10);
			if (ret > 0)
				bufpos += ret;
			if (! thread_started)
				break;
		}
	}
	if (bufpos == 0)
		return 0;
	//lt_info("%s buf_size %d bufpos %d th %d tmp %d\n", __func__, buf_size, bufpos, thread_started, tmp);
	if (bufpos > buf_size) {
		memcpy(buf, dmxbuf, buf_size);
		memmove(dmxbuf, dmxbuf + buf_size, bufpos - buf_size);
		bufpos -= buf_size;
		return buf_size;
	}
	memcpy(buf, dmxbuf, bufpos);
	tmp = bufpos;
	bufpos = 0;
	return tmp;
}

void cAudio::run()
{
	lt_info("====================== start decoder thread ================================\n");
	/* libavcodec & friends */
	av_register_all();

	AVCodec *codec;
	AVFormatContext *avfc = NULL;
	AVInputFormat *inp;
	AVFrame *frame;
	uint8_t *inbuf = (uint8_t *)av_malloc(INBUF_SIZE);
	AVPacket avpkt;
	int ret, driver;
	/* libao */
	ao_info *ai;
	// ao_device *adevice;
	// ao_sample_format sformat;
	/* resample */
	SwrContext *swr = NULL;
	uint8_t *obuf = NULL;
	int obuf_sz = 0; /* in samples */
	int obuf_sz_max = 0;
	int o_ch, o_sr; /* output channels and sample rate */
	uint64_t o_layout; /* output channels layout */
	char tmp[64] = "unknown";

	curr_pts = 0;
	av_init_packet(&avpkt);
	inp = av_find_input_format("mpegts");
	AVIOContext *pIOCtx = avio_alloc_context(inbuf, INBUF_SIZE, // internal Buffer and its size
			0,		// bWriteable (1=true,0=false)
			NULL,		// user data; will be passed to our callback functions
			_my_read,	// read callback
			NULL,		// write callback
			NULL);		// seek callback
	avfc = avformat_alloc_context();
	avfc->pb = pIOCtx;
	avfc->iformat = inp;
	avfc->probesize = 188*5;
	thread_started = true;

	if (avformat_open_input(&avfc, NULL, inp, NULL) < 0) {
		lt_info("%s: avformat_open_input() failed.\n", __func__);
		goto out;
	}
	ret = avformat_find_stream_info(avfc, NULL);
	lt_debug("%s: avformat_find_stream_info: %d\n", __func__, ret);
	if (avfc->nb_streams != 1)
	{
		lt_info("%s: nb_streams: %d, should be 1!\n", __func__, avfc->nb_streams);
		goto out;
	}
	if (avfc->streams[0]->codec->codec_type != AVMEDIA_TYPE_AUDIO)
		lt_info("%s: stream 0 no audio codec? 0x%x\n", __func__, avfc->streams[0]->codec->codec_type);

	c = avfc->streams[0]->codec;
	codec = avcodec_find_decoder(c->codec_id);
	if (!codec) {
		lt_info("%s: Codec for %s not found\n", __func__, avcodec_get_name(c->codec_id));
		goto out;
	}
	if (avcodec_open2(c, codec, NULL) < 0) {
		lt_info("%s: avcodec_open2() failed\n", __func__);
		goto out;
	}
	frame = av_frame_alloc();
	if (!frame) {
		lt_info("%s: avcodec_alloc_frame failed\n", __func__);
		goto out2;
	}
	/* output sample rate, channels, layout could be set here if necessary */
	o_ch = c->channels;		/* 2 */
	o_sr = c->sample_rate;		/* 48000 */
	o_layout = c->channel_layout;	/* AV_CH_LAYOUT_STEREO */
	if (sformat.channels != o_ch || sformat.rate != o_sr ||
	    sformat.byte_format != AO_FMT_NATIVE || sformat.bits != 16 || adevice == NULL)
	{
		driver = ao_default_driver_id();
		sformat.bits = 16;
		sformat.channels = o_ch;
		sformat.rate = o_sr;
		sformat.byte_format = AO_FMT_NATIVE;
		sformat.matrix = 0;
		if (adevice)
			ao_close(adevice);
		adevice = ao_open_live(driver, &sformat, NULL);
		ai = ao_driver_info(driver);
		lt_info("%s: changed params ch %d srate %d bits %d adevice %p\n",
			__func__, o_ch, o_sr, 16, adevice);;
		lt_info("libao driver: %d name '%s' short '%s' author '%s'\n",
				driver, ai->name, ai->short_name, ai->author);
	}
#if 0
	lt_info(" driver options:");
	for (int i = 0; i < ai->option_count; ++i)
		fprintf(stderr, " %s", ai->options[i]);
	fprintf(stderr, "\n");
#endif
	av_get_sample_fmt_string(tmp, sizeof(tmp), c->sample_fmt);
	lt_info("decoding %s, sample_fmt %d (%s) sample_rate %d channels %d\n",
		 avcodec_get_name(c->codec_id), c->sample_fmt, tmp, c->sample_rate, c->channels);
	swr = swr_alloc_set_opts(swr,
				 o_layout, AV_SAMPLE_FMT_S16, o_sr,			/* output */
				 c->channel_layout, c->sample_fmt, c->sample_rate,	/* input */
				 0, NULL);
	if (! swr) {
		lt_info("could not alloc resample context\n");
		goto out3;
	}
	swr_init(swr);
	while (thread_started) {
		int gotframe = 0;
		if (av_read_frame(avfc, &avpkt) < 0)
			break;
		avcodec_decode_audio4(c, frame, &gotframe, &avpkt);
		if (gotframe && thread_started) {
			int out_linesize;
			obuf_sz = av_rescale_rnd(swr_get_delay(swr, c->sample_rate) +
						 frame->nb_samples, o_sr, c->sample_rate, AV_ROUND_UP);
			if (obuf_sz > obuf_sz_max) {
				lt_info("obuf_sz: %d old: %d\n", obuf_sz, obuf_sz_max);
				av_free(obuf);
				if (av_samples_alloc(&obuf, &out_linesize, o_ch,
							frame->nb_samples, AV_SAMPLE_FMT_S16, 1) < 0) {
					lt_info("av_samples_alloc failed\n");
					av_free_packet(&avpkt);
					break; /* while (thread_started) */
				}
				obuf_sz_max = obuf_sz;
			}
			obuf_sz = swr_convert(swr, &obuf, obuf_sz,
					      (const uint8_t **)frame->extended_data, frame->nb_samples);
			curr_pts = av_frame_get_best_effort_timestamp(frame);
			lt_debug("%s: pts 0x%" PRIx64 " ?\n", __func__, curr_pts, curr_pts/90000.0);
			int o_buf_sz = av_samples_get_buffer_size(&out_linesize, o_ch,
								  obuf_sz, AV_SAMPLE_FMT_S16, 1);
			ao_play(adevice, (char *)obuf, o_buf_sz);
		}
		av_free_packet(&avpkt);
	}
	// ao_close(adevice); /* can take long :-( */
	av_free(obuf);
	swr_free(&swr);
 out3:
	av_frame_free(&frame);
 out2:
	avcodec_close(c);
	c = NULL;
 out:
	avformat_close_input(&avfc);
	av_free(pIOCtx->buffer);
	av_free(pIOCtx);
	lt_info("======================== end decoder thread ================================\n");
}
