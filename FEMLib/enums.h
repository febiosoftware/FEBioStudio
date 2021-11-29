#pragma once

//-----------------------------------------------------------------------------
// FE Super classes
// NOTE: The values of this enum must correspond to the super class IDs in FEBio (defined in fecore_enum.h)!
enum FESuperClass
{
	FE_MATERIAL = 4,
	FE_MATERIALPROP = 5,
	FE_DISCRETE_MATERIAL = 6,
	FE_BODY_LOAD = 7,
	FE_SURFACE_LOAD = 8,
	FE_NODAL_LOAD = 10,
	FE_CONSTRAINT = 11,
	FE_PLOTDATA = 12,
	FE_ANALYSIS = 13,
	FE_INTERFACE = 14,
	FE_ESSENTIAL_BC = 19,
	FE_INITIAL_CONDITION = 25,
	FE_VECTORGENERATOR = 31,
	FE_MAT3DGENERATOR = 32,
	FE_MESH_ADAPTOR = 36,
	FE_RIGID_CONSTRAINT = 38,
	FE_RIGID_LOAD = 39,
	FE_RIGID_CONNECTOR = 40
};

