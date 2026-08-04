#include <gtest/gtest.h>
#include <GeomLib/GPrimitive.h>
#include "tools.h"

TEST(ModifierTests, WeldObjects)
{
	GBox o1;
	GBox o2;
	Transform& T = o2.GetTransform();
	T.Translate(vec3d(1, 0, 0));

	GMultiBox mb1(&o1);
	GMultiBox mb2(&o2);

	mb1.Merge(mb2);

	// Check the properties of the welded object
	EXPECT_TOPO(mb1, 12, 20, 11, 2);
}
