/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "stdafx.h"
#include "MPEGAnimation.h"
#include <QImage>
#include <math.h>

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
    
    #if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(58, 137, 100)
        avcodec_register_all();
    #endif
    
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
    
    av_codec_context->bit_rate = 40000000;
    
    // resolution must be a multiple of 2
    av_codec_context->width = cx;
    av_codec_context->height = cy;
    
    // frames per second
    // MPEG-1/2 only supports specific FPS values, two of which are 25, and 60. 60 is the highest possible
    // In order to provide some sort of FPS control, we choose the a video framerate based on the user-
    // specified fps. We then add in dubplicate frames in order to allow

//    if(fps > 60) fps = 60;
//    m_repeatFrames = round(60/fps);
//
//    int videoFPS;
//    if(fps <= 25) videoFPS = 25;
//    else videoFPS = 60;
    int videoFPS = 25;

    av_codec_context->time_base = av_make_q(1,videoFPS);
	av_codec_context->framerate = av_make_q(videoFPS, 1);

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

    av_image_fill_arrays(yuv_frame->data, yuv_frame->linesize, buffer, AV_PIX_FMT_YUV420P, av_codec_context->width, av_codec_context->height, 1);

    return true;
}

bool CMPEGAnimation::EncodeVideo(AVFrame *frame) 
{
    int ret;

    ret = avcodec_send_frame(av_codec_context, frame);
    if (ret < 0) 
    {
        return false;
    }

    while (ret >= 0) 
    {
        ret = avcodec_receive_packet(av_codec_context, &av_packet);
        // Nothing wrong, just done or need more frames
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
        {
            return true;
        }
        else if (ret < 0) // Presumably unrecoverable errors
        {
            return false;
        }

        fwrite(av_packet.data, 1, av_packet.size, file);
        av_packet_unref(&av_packet);
    }
    return true;
}

int CMPEGAnimation::Write(QImage &im)
{
	// cannot convert rgb24 to yuv420
    if (!Rgb24ToYuv420p(im))
    {
        return false;
    }
    
    // Commenting out repeat frames as they result in choppy video in some cases
//    for(int index = 0; index < m_repeatFrames; index++)
//    {
    
	int got_packet = 0;
	av_init_packet(&av_packet);
	av_packet.data = NULL;
	av_packet.size = 0;

	fflush(stdout);

	yuv_frame->pts = m_nframe++;

    if (!EncodeVideo(yuv_frame))
    {
        return false;
    }
//	}

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

	int linesize[] = { (int)im.bytesPerLine(), 0, 0, 0, 0, 0, 0, 0 };
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
        fflush(stdout);
        EncodeVideo(NULL);
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
