#include "stdafx.h"
#include "MPEGAnimation.h"
#include <QImage>

#ifdef FFMPEG
CMPEGAnimation::CMPEGAnimation()
{
    file = NULL;
    av_codec_context = NULL;
    av_output_format = NULL;
    rgb_frame = NULL;
    yuv_frame = NULL;
    av_format_context = NULL;
}

int CMPEGAnimation::Create(const char *szfile, int cx, int cy, float fps)
{
	m_nframe = 0;
    
    avcodec_register_all();
    
    // find mpeg1 video encoder
    av_codec = avcodec_find_encoder(AV_CODEC_ID_MPEG1VIDEO);
    
    if (!av_codec)
    {
        return false;
    }
    
    // alloc an AVCodecContext
    av_codec_context = avcodec_alloc_context3(av_codec);
    
    if (!av_codec_context)
    {
        return false;
    }
    
    av_codec_context->bit_rate = 400000;
    
    // resolution must be a multiple of 2
    av_codec_context->width = cx;
    av_codec_context->height = cy;
    
    // frames per second
    av_codec_context->time_base = av_make_q(1,25);
	av_codec_context->framerate = av_make_q(25, 1);
    
    // emit one intra frame every ten frames
    av_codec_context->gop_size = 10;
    av_codec_context->max_b_frames = 1;
    av_codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
    
    // open the codec
    if (avcodec_open2(av_codec_context, av_codec, NULL))
    {
        return false;
    }
    
    file = fopen(szfile, "wb");
    
    if (!file)
    {
        return false;
    }
    
	yuv_frame = av_frame_alloc();
	if (yuv_frame == 0) return false;

	yuv_frame->format = av_codec_context->pix_fmt;
	yuv_frame->width = av_codec_context->width;
	yuv_frame->height = av_codec_context->height;
	yuv_frame->pts = 0;

	int yuv_frame_bytes = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, av_codec_context->width, av_codec_context->height, 32);

	buffer = (uint8_t *)av_malloc(yuv_frame_bytes * sizeof(uint8_t));

	avpicture_fill((AVPicture *)yuv_frame, buffer, AV_PIX_FMT_YUV420P, av_codec_context->width, av_codec_context->height);

    return true;
}

int CMPEGAnimation::Write(QImage &im)
{
    // cannot convert rgb24 to yuv420
    if (!Rgb24ToYuv420p(im))
    {
        return false;
    }
    
    int ret;
    int got_packet = 0;
	av_init_packet(&av_packet);
    av_packet.data = NULL;
    av_packet.size = 0;
        
    fflush(stdout);
        
    yuv_frame->pts = m_nframe++;
        
    // encode the pix
    ret = avcodec_encode_video2(av_codec_context, &av_packet, yuv_frame, &got_packet);
        
    if (ret < 0)
    {
        return false;
    }
        
    if (got_packet)
    {
        fwrite(av_packet.data, 1, av_packet.size, file);
        av_free_packet(&av_packet);
    }
    
    return true;
}

bool CMPEGAnimation::Rgb24ToYuv420p(QImage &im)
{
    struct SwsContext *converted_format = NULL;
    
	converted_format = sws_getCachedContext(converted_format, av_codec_context->width, av_codec_context->height, AV_PIX_FMT_BGRA, av_codec_context->width, av_codec_context->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
    
    if (!converted_format)
    {
        return false;
    }

	int linesize[] = { im.bytesPerLine(), 0, 0, 0, 0, 0, 0, 0 };
	const uint8_t* p = im.bits();
    
    sws_scale(converted_format, &p, linesize, 0, av_codec_context->height, yuv_frame->data, yuv_frame->linesize);
    
    //sws_scale(converted_format, (const uint8_t* const*)rgb_frame->data, rgb_frame->linesize, 0, av_codec_context->height, yuv_frame->data, yuv_frame->linesize);
    
    return true;
}

void CMPEGAnimation::Close()
{
    // get the delayed frames
	if (m_nframe > 0)
	{
		int got_packet;
		do
		{
			fflush(stdout);

			int ret = avcodec_encode_video2(av_codec_context, &av_packet, NULL, &got_packet);

			if (ret < 0)
			{
				break;
			}

			if (got_packet)
			{
				fwrite(av_packet.data, 1, av_packet.size, file);
				av_free_packet(&av_packet);
			}
		} while (got_packet);
	}

    // add sequence end code to have a real video file
    uint8_t endcode[] = {0, 0, 1, 0xb7};
    fwrite(endcode, 1, sizeof(endcode), file);

    // deallocating AVCodecContext
    avcodec_close(av_codec_context);
    av_free(av_codec_context);
    if (file) fclose(file);
	av_frame_free(&yuv_frame);

    file = NULL;
    av_codec_context = NULL;
    av_output_format = NULL;
    rgb_frame = NULL;
    yuv_frame = NULL;
    av_format_context = NULL;
}
#endif
