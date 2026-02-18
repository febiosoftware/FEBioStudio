/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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
#include "rhiTexture.h"
#include "rhiUtil.h"

void rhi::Texture::create(const QImage& img)
{
	QSize size = img.size();
	image = img;
	sampler.reset(m_rhi->newSampler(
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge));
	sampler->create();

	texture.reset(m_rhi->newTexture(QRhiTexture::RGBA8, size));
	texture->create();

	needsUpload = true;
}

void rhi::Texture::setImage(const QImage& img)
{
	if (texture->pixelSize() != img.size())
	{
		image = img;
		texture->setPixelSize(img.size());
		texture->create();
	}
	else image = img;
	needsUpload = true;
}

void rhi::Texture::update(QRhiResourceUpdateBatch* u)
{
	if (needsUpload)
		u->uploadTexture(texture.get(), image);

	needsUpload = false;
}

void rhi::Texture3D::create(const C3DImage& img)
{
	int W = img.Width();
	int H = img.Height();
	int D = img.Depth();
	needsUpload = false;

	QRhiTexture::Format fmt = QRhiTexture::UnknownFormat;

	int trgBytesPerVoxel = 0;
	switch (img.PixelType())
	{
	case CImage::UINT_8   : fmt = QRhiTexture::R8   ; trgBytesPerVoxel = 1; break;
	case CImage::UINT_16  : fmt = QRhiTexture::R16  ; trgBytesPerVoxel = 2; break;
	case CImage::UINT_RGB8: fmt = QRhiTexture::RGBA8; trgBytesPerVoxel = 4; break;
	default:
		assert(false);
		return;
	}

	bool b = m_rhi->isTextureFormatSupported(fmt, QRhiTexture::ThreeDimensional);
	assert(b);
	if (!b) return;

	// map data
	if (!mapData(data, img, trgBytesPerVoxel))
		return;

	slices = D;
	sliceSize = W * H * trgBytesPerVoxel;

	texture.reset(m_rhi->newTexture(fmt,
		W, H, D, 1, QRhiTexture::ThreeDimensional));
	texture->create();

	sampler.reset(m_rhi->newSampler(
		QRhiSampler::Linear,
		QRhiSampler::Linear,
		QRhiSampler::None,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge,
		QRhiSampler::ClampToEdge));

	sampler->create();

	needsUpload = true;
}

void rhi::Texture3D::upload(QRhiResourceUpdateBatch* u)
{
	const uint8_t* base = data.data();

	QRhiTextureUploadDescription desc;
	std::vector<QRhiTextureUploadEntry> entries(slices);
	for (int z = 0; z < slices; ++z) {

		const uint8_t* slicePtr = base + qint64(z) * (sliceSize);
		QRhiTextureSubresourceUploadDescription subRes = { slicePtr, (quint32) sliceSize };
		entries[z] = { z, 0, subRes };
	}
	desc.setEntries(entries.begin(), entries.end());

	u->uploadTexture(texture.get(), desc);
}

bool rhi::Texture3D::mapData(std::vector<uint8_t>& dst, const C3DImage& img, int trgBytesPerVoxel)
{
	int W = img.Width();
	int H = img.Height();
	int D = img.Depth();
	int srcBytesPerVoxel = img.BPS();

	size_t dstSize = W * H * D * trgBytesPerVoxel;
	dst.resize(dstSize);

	if (srcBytesPerVoxel == trgBytesPerVoxel)
	{
		// copy the whole thing
		memcpy(dst.data(), img.GetBytes(), dstSize);
	}
	else if (trgBytesPerVoxel > srcBytesPerVoxel)
	{
		// need to add padding
		const uint8_t* s = img.GetBytes();
		uint8_t* d = dst.data();
		size_t sliceSize = W * H;
		for (int i = 0; i < D; ++i)
		{
			for (size_t j = 0; j < sliceSize; ++j)
			{
				memcpy(d, s, srcBytesPerVoxel);
				d += trgBytesPerVoxel;
				s += srcBytesPerVoxel;
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}
