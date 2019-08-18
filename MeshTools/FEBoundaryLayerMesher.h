#pragma once
#include "FEModifier.h"

//-----------------------------------------------------------------------------
// This is a routine for creating a boundary layer on a selected
// boundary surface of a hex8/penta6 mesh.
// The layer of hex/penta elements adjacent to the selected surface is
// subdivided into a (optionally biased) mesh along the topological
// normal (not the true normal) to the selected boundary.
// If two adjacent faces of the same element are selected, this represents
// an external corner in the mesh, which requires special treatment.
// Any other combination of multiple face selections on the same element
// cannot be accommodated and the routine exits without any changes.
// Two adjacent faces form an internal corner of the mesh if corresponding
// hex elements don't share a face, but share a single common neighboring
// element which only has an edge on the selected surface. These internal
// corners are accommodated in this routine.
class FEBoundaryLayerMesher : public FEModifier
{
public:
	FEBoundaryLayerMesher();

	FEMesh* Apply(FEMesh* pm);

protected:
	void BoundaryLayer(FEMesh* pm);
};
