#include <gtest/gtest.h>
#include <MeshLib/FSElementLibrary.h>

int main(int argc, char** argv)
{
	FSElementLibrary::InitLibrary();

	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
