#include <gtest/gtest.h>
#include <GeomLib/GPrimitive.h>
#include <MeshLib/FSMesh.h>
#include <MeshTools/FEBox.h>
#include "tools.h"

TEST(MeshingTests, BoxMesh)
{
	GBox o;
	FEBoxMesher* mesher = dynamic_cast<FEBoxMesher*>(o.GetFEMesher());
	ASSERT_NE(mesher, nullptr);
	mesher->SetResolution(5, 5, 5);

	FSMesh* m = o.BuildMesh();
	EXPECT_NE(m, nullptr);
	EXPECT_MESH_TOPO(*m, 216, 150, 125);
}
