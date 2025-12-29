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
#include <GeomLib/GMeshObject.h>

namespace Post
{
	class CGLModel;
	class GLSurface;
}

class CPostObject : public GMeshObject
{
public:
	CPostObject(Post::CGLModel* glm);
	~CPostObject();

	void UpdateMesh() override;

	BOX GetBoundingBox();

	void BuildFERenderMesh() override;

public:
	void SetShellToSolid(bool b) { shellToSolid = b; }
	bool GetShellToSolid() const { return shellToSolid; }

	void SetShellReferenceSurface(int n) { shellRefSurface = n; }

public: // internal surfaces

	int InternalSurfaces() { return (int)m_innerSurface.size(); }
	Post::GLSurface& InteralSurface(int i) { return *m_innerSurface[i]; }

	void ClearInternalSurfaces();

	void BuildInternalSurfaces();

private:
	// Don't want the sections on the post side. 
	void UpdateSections() override {}

private:
	Post::CGLModel* m_glm;
	std::vector<Post::GLSurface*> m_innerSurface;

	bool shellToSolid = false;
	int shellRefSurface = 0; // 0 = mid, 1 = bottom, 2 = top
};
