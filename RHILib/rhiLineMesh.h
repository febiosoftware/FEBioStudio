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
	class LineMesh : public Mesh
	{
	public:
		LineMesh(QRhi* rhi) : rhi::Mesh(rhi) {}

		bool CreateFromGLMesh(const GLMesh* gmsh) override
		{
			submeshes.clear();

			if (gmsh == nullptr) return false;

			int NE = gmsh->Edges();
			int NV = 2 * NE;
			submeshes.emplace_back(new SubMesh(this, 0, NV));

			std::vector<Vertex> vertexData;
			vertexData.resize(NV);
			Vertex* v = vertexData.data();
			unsigned int vertexCount = 0;

			if (gmsh->EdgePartitions() == 0)
			{ 
				for (int i = 0; i < NE; ++i)
				{
					const GLMesh::EDGE& e = gmsh->Edge(i);
					for (int j = 0; j < 2; ++j, ++v) {
						GLMesh::NODE nd;
						nd.r = e.vr[j];
						nd.t = e.vt[j];
						nd.c = e.c[j];
						(*v) = nd;
					}
				}
			}
			else
			{
				unsigned int vertexCount = 0;
				for (int n = 0; n < gmsh->EdgePartitions(); ++n)
				{
					const GLMesh::EDGE_PARTITION& edge = gmsh->EdgePartition(n);
					for (int i = 0; i < edge.ne; ++i) {
						const GLMesh::EDGE& e = gmsh->Edge(i + edge.n0);
						for (int j = 0; j < 2; ++j, ++v) {
							GLMesh::NODE nd;
							nd.r = e.vr[j];
							nd.t = e.vt[j];
							nd.c = e.c[j];
							(*v) = nd;
						}
					}
					submeshes.emplace_back(new SubMesh(this, vertexCount, (unsigned int)edge.ne*2));
					vertexCount += (unsigned int)edge.ne*2;
				}
			}
			create(NV, sizeof(Vertex), vertexData.data());
			return true;
		}

	private:
		LineMesh(const LineMesh&) = delete;
		void operator = (const LineMesh&) = delete;
	};

	// Mesh for rendering wide lines. Each edge is converted to two triangles
	// (6 vertices). Every vertex stores both end points of the edge, so that
	// the vertex shader can extrude the quad in screen space.
	template <typename Vertex>
	class WideLineMesh : public Mesh
	{
	public:
		WideLineMesh(QRhi* rhi) : rhi::Mesh(rhi) {}

		bool CreateFromGLMesh(const GLMesh* gmsh) override
		{
			submeshes.clear();

			if (gmsh == nullptr) return false;

			const int NE = gmsh->Edges();
			const int NV = 6 * NE;
			submeshes.emplace_back(new SubMesh(this, 0, NV));

			std::vector<Vertex> vertexData;
			vertexData.resize(NV);
			Vertex* v = vertexData.data();

			if (gmsh->EdgePartitions() == 0)
			{
				for (int i = 0; i < NE; ++i, v += 6) addEdge(gmsh->Edge(i), v);
			}
			else
			{
				unsigned int vertexCount = 0;
				for (int n = 0; n < gmsh->EdgePartitions(); ++n)
				{
					const GLMesh::EDGE_PARTITION& edge = gmsh->EdgePartition(n);
					for (int i = 0; i < edge.ne; ++i, v += 6) addEdge(gmsh->Edge(i + edge.n0), v);
					submeshes.emplace_back(new SubMesh(this, vertexCount, (unsigned int)edge.ne * 6));
					vertexCount += (unsigned int)edge.ne * 6;
				}
			}
			create(NV, sizeof(Vertex), vertexData.data());
			return true;
		}

	private:
		static void addEdge(const GLMesh::EDGE& e, Vertex* v)
		{
			// (endpoint, side) for the two triangles of the quad
			const float q[6][2] = {
				{0.f, -1.f}, {1.f, -1.f}, {1.f, 1.f},
				{0.f, -1.f}, {1.f,  1.f}, {0.f, 1.f}
			};
			for (int k = 0; k < 6; ++k)
			{
				int j = (q[k][0] == 0.f ? 0 : 1);
				Vertex& vk = v[k];
				vk.a = e.vr[0];
				vk.b = e.vr[1];
				vk.t = e.vt[j];
				e.c[j].toFloat(vk.c);
				vk.e[0] = q[k][0];
				vk.e[1] = q[k][1];
			}
		}

	private:
		WideLineMesh(const WideLineMesh&) = delete;
		void operator = (const WideLineMesh&) = delete;
	};
}
