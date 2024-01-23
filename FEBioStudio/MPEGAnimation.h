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
#include "libswscale/swscale.h"
}

// -----------------------------------------------------------------
class CMPEGAnimation : public CAnimation
{
public:
    CMPEGAnimation();
    
public:
    int Create(const char* szfile, int cx, int cy, float fps = 10.f) override;
    int Write(QImage& im) override;
    void Close() override;
    bool IsValid() override { return (file != NULL); }
	int Frames() override { return m_nframe; };

protected:
    FILE *file; // a file pointer
    AVCodecContext *av_codec_context; // save the stream infomation
    const AVCodec *av_codec; // encoder
    AVPacket av_packet; // all frames will be dumped into avpacket for muxing
    AVOutputFormat *av_output_format; // the output format for muxing
    AVFrame *rgb_frame;
    AVFrame *yuv_frame; // save the yuv frame data
    AVFormatContext *av_format_context;
    uint8_t *buffer;
	int		m_nframe;	// frame index
    
private:
    bool Rgb24ToYuv420p(QImage &im);
    bool EncodeVideo(AVFrame *frame);

    int m_repeatFrames;
};
#endif
