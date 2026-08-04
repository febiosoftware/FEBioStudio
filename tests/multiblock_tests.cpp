#include <gtest/gtest.h>
#include <GeomLib/GPrimitive.h>
#include <GeomLib/GMultiBox.h>
#include "tools.h"

TEST(MultiBlockTests, CreateMultiBoxFromBox)
{
	GBox o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 8, 12, 6, 1);
}

TEST(MultiBlockTests, CreateMultiBoxFromCylinder)
{
	GCylinder o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 34, 73, 52, 12);
}


TEST(MultiBlockTests, CreateMultiBoxFromTube)
{
	GTube o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 16, 32, 20, 4);
}

TEST(MultiBlockTests, CreateMultiBoxFromSphere)
{
	GSphere o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 53, 128, 108, 32);
}

TEST(MultiBlockTests, CreateMultiBoxFromCone)
{
	GCone o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 34, 73, 52, 12);
}

//TEST(MultiBlockTests, CreateMultiBoxFromTruncatedEllipsoid)
//{
//	GTruncatedEllipsoid o;
//	GMultiBox mb(&o);
//	EXPECT_TOPO(12, 20, 12, 1);
//}

TEST(MultiBlockTests, CreateMultiBoxFromTorus)
{
	GTorus o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 68, 180, 160, 48);
}

//TEST(MultiBlockTests, CreateMultiBoxFromSlice)
//{
//	GSlice o;
//	GMultiBox mb(&o);
//	EXPECT_TOPO(mb, 6, 9, 5, 1);
//}

TEST(MultiBlockTests, CreateMultiBoxFromSolidArc)
{
	GSolidArc o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 8, 12, 6, 1);
}

TEST(MultiBlockTests, CreateMultiBoxFromDogBone)
{
	GQuartDogBone o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 28, 49, 30, 6);
}

TEST(MultiBlockTests, CreateMultiBoxFromCylinderInBox)
{
	GCylinderInBox o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 18, 32, 20, 4);
}

TEST(MultiBlockTests, CreateMultiBoxFromSphereInBox)
{
	GSphereInBox o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 53, 122, 96, 24);
}

TEST(MultiBlockTests, CreateMultiBoxFromHollowSphere)
{
	GHollowSphere o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 53, 122, 96, 24);
}

TEST(MultiBlockTests, CreateMultiBoxFromBoxInBox)
{
	GBoxInBox o;
	GMultiBox mb(&o);
	EXPECT_TOPO(mb, 16, 32, 24, 6);
}