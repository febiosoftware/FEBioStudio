#pragma once
#include <gtest/gtest.h>
#include <GeomLib/GObject.h>
#include <MeshLib/FSMesh.h>

inline void EXPECT_TOPO(const GObject& mb, int nodes, int edges, int faces, int parts)
{
	EXPECT_EQ(mb.Nodes(), nodes);
	EXPECT_EQ(mb.Edges(), edges);
	EXPECT_EQ(mb.Faces(), faces);
	EXPECT_EQ(mb.Parts(), parts);
}

inline void EXPECT_MESH_TOPO(const FSMesh& m, int nodes, int faces, int elems)
{
	EXPECT_EQ(m.Nodes(), nodes);
	EXPECT_EQ(m.Faces(), faces);
	EXPECT_EQ(m.Elements(), elems);
}
