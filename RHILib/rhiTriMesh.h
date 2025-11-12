#pragma once
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
#include "rhiMesh.h"

namespace rhi {

	template <typename Vertex>
	class TriMesh : public Mesh
	{
	public:
		TriMesh(QRhi* rhi) : rhi::Mesh(rhi) {}

		bool CreateFromGLMesh(const GLMesh* gmsh) override
		{
			submeshes.clear();

			assert(gmsh);
			if (gmsh == nullptr) return false;

			assert(gmsh->Partitions() > 0);
			if (gmsh->Partitions() == 0) return false;

			int NF = gmsh->Faces();
			if (NF == 0) return false;

			int NV = 3 * NF;
			std::vector<Vertex> vertexData;
			vertexData.resize(NV);
			Vertex* v = vertexData.data();
			unsigned int vertexCount = 0;

			// first sub-mesh is for the entire mesh
			submeshes.emplace_back(std::make_unique<SubMesh>(this, 0, NV));

			// add all the partitions
			for (size_t partition = 0; partition < gmsh->Partitions(); ++partition)
			{
				const GLMesh::PARTITION& part = gmsh->Partition(partition);
				for (int i = 0; i < part.nf; ++i)
				{
					const GLMesh::FACE& f = gmsh->Face(i + part.n0);
					for (int j = 0; j < 3; ++j, ++v)
					{
						GLMesh::NODE nd;
						nd.r = f.vr[j];
						nd.n = f.vn[j];
						nd.c = f.c[j];
						nd.t = f.t[j];
						(*v) = nd;
					}
				}
				submeshes.emplace_back(std::make_unique<SubMesh>(this, vertexCount, 3 * part.nf));

				vertexCount += 3*part.nf;
			}

			// create the vertex buffer
			create(NV, sizeof(Vertex), vertexData.data());

			return true;
		}

	private:
		TriMesh(const TriMesh&) = delete;
		void operator = (const TriMesh&) = delete;
	};
}
