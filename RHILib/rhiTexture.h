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
#pragma once
#include <rhi/qrhi.h>
#include <GLLib/GLTexture3D.h>
#include <QImage>

namespace rhi {

	class Texture
	{
	public:
		Texture(QRhi* rhi) : m_rhi(rhi){}

		void create(const QImage& img);

		void update(QRhiResourceUpdateBatch* u);

		void setImage(const QImage& img);

		const QImage& getImage() const { return image; }

		bool isValid() const { return !image.isNull(); }

	public:
		std::unique_ptr<QRhiTexture> texture;
		std::unique_ptr<QRhiSampler> sampler;

	private:
		QRhi* m_rhi = nullptr;
		bool needsUpload = false;
		QImage image;
		Texture(const Texture&) = delete;
		void operator = (const Texture&) = delete;
	};

	class Texture3D
	{
	public:
		Texture3D(QRhi* rhi) : m_rhi(rhi) {}

		void create(const C3DImage& img);

		void upload(QRhiResourceUpdateBatch* u);

	public:
		QRhi* m_rhi = nullptr;
		std::unique_ptr<QRhiTexture> texture;
		std::unique_ptr<QRhiSampler> sampler;
		bool needsUpload = false;
		QRhiTexture::Format fmt = QRhiTexture::UnknownFormat;

	private:
		bool mapData(std::vector<unsigned char>& d, const C3DImage& s, int trgBytesPerVoxel);

	private:
		Texture3D(const Texture3D&) = delete;
		void operator = (const Texture3D&) = delete;
		std::vector<uint8_t> data; // copy of image data (released after update)
		size_t slices = 0;
		size_t sliceSize = 0;
	};
}
