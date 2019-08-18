#pragma once

namespace Post {

class GLTexture1D
{
public:
	GLTexture1D();
	GLTexture1D(const GLTexture1D& tex);

	void SetTexture(unsigned char* pb);

	void MakeCurrent();

	int Size();

	unsigned char* GetBytes();

	void Update();

protected:
	int		m_n;
	unsigned int		m_texID;
	unsigned char m_pb[3 * 1024];
	bool	m_bupdate;
};
}
