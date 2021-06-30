#pragma once

//-----------------------------------------------------------------------------
// FE Super classes
// NOTE: The values of this enum must correspond to the super class IDs in FEBio (defined in fecore_enum.h)!
enum FESuperClass
{
	FE_MATERIAL = 4,
	FE_MATERIALPROP = 5,
	FE_BODY_LOAD = 6,
	FE_SURFACE_LOAD = 7,
	FE_NODAL_LOAD = 9,
	FE_CONSTRAINT = 10,
	FE_PLOTDATA = 11,
	FE_ANALYSIS = 12,
	FE_INTERFACE = 13,
	FE_ESSENTIAL_BC = 18,
	FE_INITIAL_CONDITION = 24,
	FE_MESH_ADAPTOR = 35,
	FE_RIGID_CONSTRAINT,
	FE_RIGID_CONNECTOR
};

