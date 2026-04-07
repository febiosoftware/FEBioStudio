#include <gtest/gtest.h>
#include <GeomLib/GPrimitive.h>
#include "tools.h"

TEST(PrimitiveTests, CreateBox)
{
	GBox o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 8, 12, 6, 1);
}

TEST(PrimitiveTests, CreateCylinder)
{
	GCylinder o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 10, 12, 6, 1);
}

TEST(PrimitiveTests, CreateTube)
{
	GTube o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 18, 32, 16, 1);
}

TEST(PrimitiveTests, CreateSphere)
{
	GSphere o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 6, 12, 8, 1);
}

TEST(PrimitiveTests, CreateCone)
{
	GCone o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 8, 12, 6, 1);
}

TEST(PrimitiveTests, CreateTruncatedEllipsoid)
{
	GTruncatedEllipsoid o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 10, 20, 12, 1);
}

TEST(PrimitiveTests, CreateTorus)
{
	GTorus o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 20, 32, 16, 1);
}

TEST(PrimitiveTests, CreateSlice)
{
	GSlice o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 6, 9, 5, 1);
}

TEST(PrimitiveTests, CreateSolidArc)
{
	GSolidArc o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 10, 12, 6, 1);
}

TEST(PrimitiveTests, CreateHexagon)
{
	GHexagon o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 12, 18, 8, 1);
}

TEST(PrimitiveTests, CreateDogBone)
{
	GQuartDogBone o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 16, 21, 9, 1);
}

TEST(PrimitiveTests, CreateCylinderInBox)
{
	GCylinderInBox o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 18, 32, 16, 1);
}

TEST(PrimitiveTests, CreateSphereInBox)
{
	GSphereInBox o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 16, 24, 12, 1);
}

TEST(PrimitiveTests, CreateHollowSphere)
{
	GHollowSphere o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 12, 24, 16, 1);
}

TEST(PrimitiveTests, CreateBoxInBox)
{
	GBoxInBox o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 16, 24, 12, 1);
}

TEST(PrimitiveTests, CreateThinTube)
{
	GThinTube o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 10, 12, 4, 1);
}

TEST(PrimitiveTests, CreatePatch)
{
	GPatch o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 4, 4, 1, 1);
}

TEST(PrimitiveTests, CreateDisc)
{
	GDisc o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 5, 8, 4, 1);
}

TEST(PrimitiveTests, CreateRing)
{
	GRing o;
	EXPECT_NO_THROW(o.Update());
	EXPECT_TOPO(o, 9, 12, 4, 1);
}
