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

#pragma once
#include "Animation.h"

#ifdef FFMPEG
extern "C"
{
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libswscale/swscale.h"
}

// -----------------------------------------------------------------
// Writes H.264 video into an MP4 container.
//
// The class name is historical. It used to emit a raw MPEG-1 elementary
// stream: encoder packets fwrite() straight to a FILE* with a sequence-end
// code appended and no container at all. That produced a file with no
// timestamps and no index, which only ffmpeg-based players would open —
// QuickTime showed an empty window. Everything now goes through libavformat.
// -----------------------------------------------------------------
class CMPEGAnimation : public CAnimation
{
public:
    CMPEGAnimation();
    ~CMPEGAnimation() override;

public:
    int Create(const char* szfile, int cx, int cy, float fps = 10.f) override;
    int Write(QImage& im) override;
    void Close() override;
    bool IsValid() override { return (av_format_context != nullptr); }
	int Frames() override { return m_nframe; };

protected:
    AVFormatContext *av_format_context;  // the MP4 muxer
    AVStream        *av_stream;          // the single video stream
    AVCodecContext  *av_codec_context;   // H.264 encoder state
    const AVCodec   *av_codec;           // the encoder itself
    AVPacket        *av_packet;          // reusable output packet
    AVFrame         *yuv_frame;          // reusable YUV420P input frame
    struct SwsContext *sws_context;      // cached RGBA -> YUV420P converter
	int		m_nframe;	// frame index

private:
    bool Rgb24ToYuv420p(QImage &im);
    bool EncodeVideo(AVFrame *frame);

    bool m_headerWritten;   // av_write_trailer is only legal if it was
};
#endif
