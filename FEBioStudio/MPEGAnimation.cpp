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
    av_format_context = nullptr;
    av_stream         = nullptr;
    av_codec_context  = nullptr;
    av_codec          = nullptr;
    av_packet         = nullptr;
    yuv_frame         = nullptr;
    sws_context       = nullptr;
    m_nframe          = 0;
    m_headerWritten   = false;
}

CMPEGAnimation::~CMPEGAnimation()
{
    // Close() is idempotent; this only matters if the caller forgot.
    Close();
}

int CMPEGAnimation::Create(const char *szfile, int cx, int cy, float fps)
{
    Close();
    m_nframe = 0;

    if (fps <= 0.f) fps = 10.f;

    // H.264 in YUV420P needs even dimensions for the chroma planes.
    cx &= ~1;
    cy &= ~1;
    if ((cx <= 0) || (cy <= 0)) return false;

    // Force the MP4 muxer by NAME rather than letting libavformat guess from
    // the filename. If the user types "movie.mpg" in the save dialog, guessing
    // would select the MPEG program-stream muxer, which cannot carry H.264 —
    // the muxer would be created and then fail at write_header.
    if (avformat_alloc_output_context2(&av_format_context, NULL, "mp4", szfile) < 0) return false;
    if (av_format_context == nullptr) return false;

    av_codec = avcodec_find_encoder(AV_CODEC_ID_H264);
    if (!av_codec) return false;

    av_stream = avformat_new_stream(av_format_context, NULL);
    if (!av_stream) return false;

    av_codec_context = avcodec_alloc_context3(av_codec);
    if (!av_codec_context) return false;

    av_codec_context->width   = cx;
    av_codec_context->height  = cy;
    av_codec_context->pix_fmt = AV_PIX_FMT_YUV420P;

    // Unlike MPEG-1, H.264 accepts an arbitrary frame rate, so the user's
    // requested fps is honoured directly instead of being forced to 25 and
    // padded with duplicate frames.
    AVRational tb = av_d2q(1.0 / (double)fps, 100000);
    av_codec_context->time_base = tb;
    av_codec_context->framerate = av_inv_q(tb);
    av_stream->time_base        = tb;

    av_codec_context->gop_size     = 12;
    av_codec_context->max_b_frames = 2;

    // MP4 stores SPS/PPS in the container header, not inline in the stream.
    if (av_format_context->oformat->flags & AVFMT_GLOBALHEADER)
        av_codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    // Constant-quality rather than the old fixed 40 Mbit/s: screen recordings
    // of a 3D viewport are mostly flat colour and compress far better than a
    // fixed bitrate assumes. crf 18 is visually lossless for this material.
    av_opt_set(av_codec_context->priv_data, "preset", "medium", 0);
    av_opt_set(av_codec_context->priv_data, "crf",    "18",     0);

    if (avcodec_open2(av_codec_context, av_codec, NULL) < 0) return false;
    if (avcodec_parameters_from_context(av_stream->codecpar, av_codec_context) < 0) return false;

    if (!(av_format_context->oformat->flags & AVFMT_NOFILE))
    {
        if (avio_open(&av_format_context->pb, szfile, AVIO_FLAG_WRITE) < 0) return false;
    }

    if (avformat_write_header(av_format_context, NULL) < 0) return false;
    m_headerWritten = true;

    av_packet = av_packet_alloc();
    if (!av_packet) return false;

    yuv_frame = av_frame_alloc();
    if (!yuv_frame) return false;
    yuv_frame->format = AV_PIX_FMT_YUV420P;
    yuv_frame->width  = cx;
    yuv_frame->height = cy;
    // Let libavutil own the frame buffer. The old code hand-rolled this with
    // av_malloc + av_image_fill_arrays, which cannot be reference-counted and
    // therefore cannot be made writable while the encoder still holds it.
    if (av_frame_get_buffer(yuv_frame, 32) < 0) return false;

    return true;
}

bool CMPEGAnimation::EncodeVideo(AVFrame *frame)
{
    if ((av_codec_context == nullptr) || (av_packet == nullptr) ||
        (av_format_context == nullptr) || (av_stream == nullptr))
    {
        return false;
    }

    int ret = avcodec_send_frame(av_codec_context, frame);
    if (ret < 0) return false;

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(av_codec_context, av_packet);
        // Not an error: the encoder wants more input, or the drain is done.
        if ((ret == AVERROR(EAGAIN)) || (ret == AVERROR_EOF)) return true;
        if (ret < 0) return false;

        // The encoder stamps packets in its own time base; the muxer expects
        // the stream's. With B-frames these differ, and skipping this step
        // produces a file that plays at the wrong speed or not at all.
        av_packet_rescale_ts(av_packet, av_codec_context->time_base, av_stream->time_base);
        av_packet->stream_index = av_stream->index;

        // Takes ownership of the packet's contents and unrefs it for us.
        ret = av_interleaved_write_frame(av_format_context, av_packet);
        av_packet_unref(av_packet);
        if (ret < 0) return false;
    }
    return true;
}

int CMPEGAnimation::Write(QImage &im)
{
    if (yuv_frame == nullptr) return false;

    // The encoder may still reference the previous frame (B-frames), so ask
    // for a private copy before overwriting the planes.
    if (av_frame_make_writable(yuv_frame) < 0) return false;

    if (!Rgb24ToYuv420p(im)) return false;

    yuv_frame->pts = m_nframe++;

    if (!EncodeVideo(yuv_frame)) return false;

    return true;
}

bool CMPEGAnimation::Rgb24ToYuv420p(QImage &im)
{
    if (im.isNull() || (av_codec_context == nullptr) || (yuv_frame == nullptr))
    {
        return false;
    }

    // Guarantee the memory layout actually matches AV_PIX_FMT_RGBA.
    // QImage::Format_ARGB32 / Format_RGB32 are BGRA in memory on little-endian
    // machines, so without this the red and blue channels come out swapped.
    // QImage is implicitly shared, so this is free when no conversion is needed.
    const QImage src = (im.format() == QImage::Format_RGBA8888)
                     ? im
                     : im.convertToFormat(QImage::Format_RGBA8888);
    if (src.isNull()) return false;

    const int srcW = src.width();
    const int srcH = src.height();
    if ((srcW <= 0) || (srcH <= 0)) return false;

    // Describe the source using the QImage's OWN dimensions, never the
    // encoder's. The captured frame is not guaranteed to match the size handed
    // to Create() -- on a HiDPI display the RHI capture comes back at a
    // different pixel size. Passing the encoder height as srcSliceH makes
    // sws_scale walk off the end of the QImage buffer and segfault.
    sws_context = sws_getCachedContext(sws_context,
        srcW, srcH, AV_PIX_FMT_RGBA,
        av_codec_context->width, av_codec_context->height, AV_PIX_FMT_YUV420P,
        SWS_BICUBIC, NULL, NULL, NULL);

    if (!sws_context) return false;

    const uint8_t* srcData[4]   = { src.constBits(), nullptr, nullptr, nullptr };
    int            srcStride[4] = { (int)src.bytesPerLine(), 0, 0, 0 };

    sws_scale(sws_context, srcData, srcStride, 0, srcH,
              yuv_frame->data, yuv_frame->linesize);

    return true;
}

void CMPEGAnimation::Close()
{
    // Flush the encoder's internal queue. With max_b_frames > 0 several frames
    // are still held back at this point; without the drain the tail of the
    // recording is silently lost.
    if ((av_codec_context != nullptr) && (m_nframe > 0))
    {
        EncodeVideo(NULL);
    }

    // Writes the moov atom. Without it the MP4 has no index and no player will
    // open it -- this is the step the old elementary-stream code had no
    // equivalent for.
    if ((av_format_context != nullptr) && m_headerWritten)
    {
        av_write_trailer(av_format_context);
        m_headerWritten = false;
    }

    if (av_codec_context) avcodec_free_context(&av_codec_context);
    if (av_packet)        av_packet_free(&av_packet);
    if (yuv_frame)        av_frame_free(&yuv_frame);

    if (sws_context)
    {
        sws_freeContext(sws_context);
        sws_context = nullptr;
    }

    if (av_format_context)
    {
        if (!(av_format_context->oformat->flags & AVFMT_NOFILE) && av_format_context->pb)
        {
            avio_closep(&av_format_context->pb);
        }
        avformat_free_context(av_format_context);
        av_format_context = nullptr;
    }

    av_stream = nullptr;
    av_codec  = nullptr;
}
#endif
