#include "Animation.h"

struct GifWriter;

class CGIFAnimation : public CAnimation
{
public:
	CGIFAnimation();
	~CGIFAnimation();
	int Create(const char* szfile, int cx, int cy, float fps = 10.f) override;
	int Write(QImage& im) override;
	void Close() override;
	bool IsValid() override;
	int Frames() override { return m_ncnt; }

private:
	GifWriter* g;
	int	m_ncnt;
	int	m_nx;
	int	m_ny;
	int m_delay;
};

