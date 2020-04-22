#pragma once
#include <PostLib/Animation.h>

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
    AVCodec *av_codec; // encoder
    AVPacket av_packet; // all frames will be dumped into avpacket for muxing
    AVOutputFormat *av_output_format; // the output format for muxing
    AVFrame *rgb_frame;
    AVFrame *yuv_frame; // save the yuv frame data
    AVFormatContext *av_format_context;
    uint8_t *buffer;
	int		m_nframe;	// frame index
    
private:
    bool Rgb24ToYuv420p(QImage &im);

    int m_repeatFrames;
};
#endif
