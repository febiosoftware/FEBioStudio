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
#include "GLPlot.h"
#include <PostLib/FEPostModel.h>

namespace Post {

	class GLPlotObjectGlyph : public CGLPlot
	{
	public:
		GLPlotObjectGlyph(FEPostModel::PointObject* po) : m_po(po) {}

	protected:
		FEPostModel::PointObject* m_po;
	};

	class GLPlotObjectVector : public GLPlotObjectGlyph
	{
		enum {
			DATA_FIELD,
			CLIP,
			SHOW_HIDDEN,
			GLYPH,
			GLYPH_COLOR,
			SOLID_COLOR,
			AUTO_SCALE,
			SCALE,
			ASPECT_RATIO,
		};

		// glyph types	
		enum Glyph_Type {
			GLYPH_ARROW,
			GLYPH_CONE,
			GLYPH_CYLINDER,
			GLYPH_SPHERE,
			GLYPH_BOX,
			GLYPH_LINE
		};

		// glyph color types
		enum Glyph_Color_Type {
			GLYPH_COL_SOLID,
			GLYPH_COL_ORIENT
		};

	public:
		GLPlotObjectVector(FEPostModel::PointObject* po);

		void Render(GLRenderEngine& re, GLContext& rc) override;

		void Update(int ntime, float dt, bool breset) override;

	private:
		void RenderVector(GLRenderEngine& re, const vec3d& r, vec3f v);

	private:
		vec3f m_val;
	};
}
