#include "FEModel.h"
#include "constants.h"
#include "FEMeshData_T.h"
using namespace Post;
extern int FT_HEX[6][4];
extern int FT_TET[4][4];
extern int FT_PENTA[5][4];
extern int FT_PENTA15[5][8];
extern int FT_HEX20[6][8];
extern int FT_HEX27[6][9];
extern int FT_TET10[4][6];
extern int FT_TET15[4][7];
extern int FT_TET20[4][10];

//-----------------------------------------------------------------------------
// extract a component from a vector
float component(const vec3f& v, int n)
{
	float g = 0.f;
	switch(n)
	{
	case 0: g = v.x; break;
	case 1: g = v.y; break;
	case 2: g = v.z; break;
	case 3: g = sqrt(v.x*v.x + v.y*v.y); break;
	case 4: g = sqrt(v.y*v.y + v.z*v.z); break;
	case 5: g = sqrt(v.x*v.x + v.z*v.z); break;
	case 6: g = sqrt(v.x*v.x + v.y*v.y + v.z*v.z); break;
	}
	return g;
}

float component2(const vec3f& v, int n)
{
	float g = 0.f;
	switch (n)
	{
	case 0: g = v.x; break;
	case 1: g = v.y; break;
	case 2: g = v.z; break;
	case 3: g = sqrt(v.x*v.x + v.y*v.y + v.z*v.z); break;
	}
	return g;
}


//-----------------------------------------------------------------------------
// extract a component from a Mat3d
float component(const Mat3d& m, int n)
{
	float g = 0.f;
	switch (n)
	{
	case 0: g = (float) m(0,0); break;
	case 1: g = (float) m(0,1); break;
	case 2: g = (float) m(0,2); break;
	case 3: g = (float) m(1,0); break;
	case 4: g = (float) m(1,1); break;
	case 5: g = (float) m(1,2); break;
	case 6: g = (float) m(2,0); break;
	case 7: g = (float) m(2,1); break;
	case 8: g = (float) m(2,2); break;
	default:
		assert(false);
	}
	return g;
}

//-----------------------------------------------------------------------------
// extract a component from a Mat3d
float component(const mat3f& m, int n)
{
	float g = 0.f;
	switch (n)
	{
	case 0: g = m(0,0); break;
	case 1: g = m(0,1); break;
	case 2: g = m(0,2); break;
	case 3: g = m(1,0); break;
	case 4: g = m(1,1); break;
	case 5: g = m(1,2); break;
	case 6: g = m(2,0); break;
	case 7: g = m(2,1); break;
	case 8: g = m(2,2); break;
	default:
		assert(false);
	}
	return g;
}

//-----------------------------------------------------------------------------
// extract a component from a tensor
float component(const mat3fs& m, int n)
{
	float g = 0.f;
	switch (n)
	{
	case 0: g = m.x; break;
	case 1: g = m.y; break;
	case 2: g = m.z; break;
	case 3: g = m.xy; break;
	case 4: g = m.yz; break;
	case 5: g = m.xz; break;
	case 6: g = m.von_mises(); break;
	case 7: { float p[3]; m.Principals(p); g = p[0]; } break;
	case 8: { float p[3]; m.Principals(p); g = p[1]; } break;
	case 9: { float p[3]; m.Principals(p); g = p[2]; } break;
	case 10: { float p[3]; m.DeviatoricPrincipals(p); g = p[0]; } break;
	case 11: { float p[3]; m.DeviatoricPrincipals(p); g = p[1]; } break;
	case 12: { float p[3]; m.DeviatoricPrincipals(p); g = p[2]; } break;
	case 13: g = m.MaxShear(); break;
	default:
		assert(false);
	}
	return g;
}

//-----------------------------------------------------------------------------
// extract a component from a tensor
float component(const mat3fd& m, int n)
{
	float g = 0.f;
	switch (n)
	{
	case 0: g = m.x; break;
	case 1: g = m.y; break;
	case 2: g = m.z; break;
	default:
		assert(false);
	}
	return g;
}

//-----------------------------------------------------------------------------
// extract a component from a fourth-order tensor
float component(const tens4fs& m, int n)
{
	float g = 0.f;
    assert((n >= 0) && (n<21));
    g = m.d[n];
	return g;
}

//-----------------------------------------------------------------------------
bool FEModel::IsValidFieldCode(int nfield, int nstate)
{
	// check the state number first
	if ((nstate < 0) || (nstate >= GetStates())) return false;

	// get the state info
	FEState& state = *m_State[nstate];

	// get the data field
	int ndata = FIELD_CODE(nfield);

	// check the field sizes
	if (ndata < 0) return false;
	if (ndata >= state.m_Data.size()) return false;

	// if we get here, everything looks good
	return true;
}

//-----------------------------------------------------------------------------
// Evaluate a data field at a particular time
bool FEModel::Evaluate(int nfield, int ntime, bool breset)
{
	// get the state data 
	FEState& state = *m_State[ntime];
	FEMeshBase* mesh = state.GetFEMesh();
	if (mesh->Nodes() == 0) return false;

	// make sure that we have to reevaluate
	if ((state.m_nField != nfield) || breset)
	{
		// store the field variable
		state.m_nField = nfield;

		if      (IS_NODE_FIELD(nfield)) EvalNodeField(ntime, nfield);
		else if (IS_ELEM_FIELD(nfield)) EvalElemField(ntime, nfield);
		else if (IS_FACE_FIELD(nfield)) EvalFaceField(ntime, nfield);
//		else assert(false);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Evaluate a nodal field
void FEModel::EvalNodeField(int ntime, int nfield)
{
	m_ntime = ntime;

	assert(IS_NODE_FIELD(nfield));

	// get the state data 
	FEState& state = *m_State[ntime];
	FEMeshBase* mesh = state.GetFEMesh();

	// first, we evaluate all the nodes
	int i, j;
	for (i=0; i<mesh->Nodes(); ++i)
	{
		FENode& node = mesh->Node(i);
		NODEDATA& d = state.m_NODE[i];
		d.m_val = 0;
		d.m_ntag = 0;
		if (node.IsEnabled()) EvaluateNode(i, ntime, nfield, d);
	}

	// Next, we project the nodal data onto the faces
	ValArray& faceData = state.m_FaceData;
	for (i=0; i<mesh->Faces(); ++i)
	{
		FEFace& f = mesh->Face(i);
		FACEDATA& d = state.m_FACE[i];
		d.m_val = 0.f;
		d.m_ntag = 0;
		if (f.IsEnabled())
		{
			d.m_ntag = 1;
			for (j=0; j<f.Nodes(); ++j) { float val = state.m_NODE[f.node[j]].m_val; faceData.value(i, j) = val; d.m_val += val; }
			d.m_val /= (float) f.Nodes();
		}
	}

	// Finally, we project the nodal data onto the elements
	ValArray& elemData = state.m_ElemData;
	for (i=0; i<mesh->Elements(); ++i)
	{
		FEElement& e = mesh->Element(i);
		ELEMDATA& d = state.m_ELEM[i];
		d.m_val = 0.f;
		d.m_state &= ~StatusFlags::ACTIVE;
		e.Deactivate();
		if (e.IsEnabled())
		{
			d.m_state |= StatusFlags::ACTIVE;
			e.Activate();
			for (j=0; j<e.Nodes(); ++j) { float val = state.m_NODE[e.m_node[j]].m_val; elemData.value(i,j) = val; d.m_val += val; }
			d.m_val /= (float) e.Nodes();
		}
	}
}

//-----------------------------------------------------------------------------
// Evaluate a face field variable
void FEModel::EvalFaceField(int ntime, int nfield)
{
	m_ntime = ntime;

	assert(IS_FACE_FIELD(nfield));

	// get the state data 
	FEState& state = *m_State[ntime];
	FEMeshBase* mesh = state.GetFEMesh();

	// get the data ID
	int ndata = FIELD_CODE(nfield);
	assert((ndata >= 0) && (ndata < state.m_Data.size()));

	// get the component
	int ncomp = FIELD_COMP(nfield);

	FEMeshData& rd = state.m_Data[ndata];
	Data_Format fmt = rd.GetFormat();

	// for float/node face data we evaluate the nodal values directly.
	if ((rd.GetType() == DATA_FLOAT) && (fmt == DATA_NODE))
	{
		// clear node data
		for (int i=0; i<mesh->Nodes(); ++i)
		{
			state.m_NODE[i].m_val = 0.f;
			state.m_NODE[i].m_ntag = 0;
		}

		// get the data field
		FEFaceData_T<float, DATA_NODE>& df = dynamic_cast<FEFaceData_T<float, DATA_NODE>&>(rd);

		// evaluate nodes and faces
		float tmp[FEGenericElement::MAX_NODES] = {0.f};
		for (int i=0; i<mesh->Faces(); ++i)
		{
			FEFace& face = mesh->Face(i);
			state.m_FACE[i].m_val = 0.f;
			state.m_FACE[i].m_ntag = 0;
			if (df.active(i))
			{
				df.eval(i, tmp);

				float avg = 0.f;
				for (int j = 0; j<face.Nodes(); ++j)
				{
					avg += tmp[j];
					state.m_NODE[face.node[j]].m_val = tmp[j];
					state.m_NODE[face.node[j]].m_ntag = 1;
				}

				state.m_FACE[i].m_val = avg / face.Nodes();
				state.m_FACE[i].m_ntag = 1;
			}
		}
	}
	else
	{
		// first evaluate all faces
		float data[FEFace::MAX_NODES], val;
		int i, j;
		for (i=0; i<mesh->Faces(); ++i)
		{
			FEFace& f = mesh->Face(i);
			state.m_FACE[i].m_val = 0.f;
			state.m_FACE[i].m_ntag = 0;
			if (f.IsEnabled()) 
			{
				if (EvaluateFace(i, ntime, nfield, data, val))
				{
					state.m_FACE[i].m_ntag = 1;
					state.m_FACE[i].m_val = val;
					for (int j=0; j<f.Nodes(); ++j) state.m_FaceData.value(i, j) = data[j];
				}
			}
		}

		// now evaluate the nodes
		ValArray& faceData = state.m_FaceData;
		for (i=0; i<mesh->Nodes(); ++i)
		{
			NODEDATA& node = state.m_NODE[i];
			vector<NodeFaceRef>& nfl = mesh->NodeFaceList(i);
			node.m_val = 0.f; 
			node.m_ntag = 0;
			int n = 0;
			for (j=0; j<(int) nfl.size(); ++j)
			{
				FACEDATA& f = state.m_FACE[nfl[j].first];
				if (f.m_ntag > 0)
				{
					node.m_val += faceData.value(nfl[j].first, nfl[j].second);
					++n;
				}
			}
			if (n > 0)
			{
				node.m_val /= (float) n;
				node.m_ntag = 1;
			}
		}
	}

	// evaluate the elements (to zero)
	// Face data is not projected onto the elements
	for (int i=0; i<mesh->Elements(); ++i) 
	{
		FEElement& el = mesh->Element(i);
		el.Deactivate();
		state.m_ELEM[i].m_val = 0.f;
		state.m_ELEM[i].m_state &= ~StatusFlags::ACTIVE;
	}
}

//-----------------------------------------------------------------------------
// Evaluate an Element field
void FEModel::EvalElemField(int ntime, int nfield)
{
	m_ntime = ntime;

	assert(IS_ELEM_FIELD(nfield));

	// get the state data 
	FEState& state = *m_State[ntime];
	FEMeshBase* mesh = state.GetFEMesh();

	// first evaluate all elements
	float data[FEGenericElement::MAX_NODES] = {0.f};
	float val;
	for (int i=0; i<mesh->Elements(); ++i)
	{
		FEElement& el = mesh->Element(i);
		state.m_ELEM[i].m_val = 0.f;
		state.m_ELEM[i].m_state &= ~StatusFlags::ACTIVE;
		el.Deactivate();
		if (el.IsEnabled()) 
		{
			if (EvaluateElement(i, ntime, nfield, data, val))
			{
				state.m_ELEM[i].m_state |= StatusFlags::ACTIVE;
				state.m_ELEM[i].m_val = val;
				el.Activate();
				int ne = el.Nodes();
				for (int j=0; j<ne; ++j) state.m_ElemData.value(i, j) = data[j];
			}
		}
	}

	// now evaluate the nodes
	ValArray& elemData = state.m_ElemData;
	for (int i=0; i<mesh->Nodes(); ++i)
	{
		FENode& node = mesh->Node(i);
		state.m_NODE[i].m_val = 0.f;
		state.m_NODE[i].m_ntag = 0;
		if (node.IsEnabled())
		{
			vector<NodeElemRef>& nel = mesh->NodeElemList(i);
			int m = (int) nel.size(), n=0;
			float val = 0.f;
			for (int j=0; j<m; ++j)
			{
				ELEMDATA& e = state.m_ELEM[nel[j].first];
				if (e.m_state & StatusFlags::ACTIVE)
				{
					val += elemData.value(nel[j].first, nel[j].second);
					++n;
				}
			}
			if (n != 0) 
			{
				state.m_NODE[i].m_val = val / (float) n;
				state.m_NODE[i].m_ntag = 1;
			}
		}
	}

	// evaluate faces
	ValArray& fd = state.m_FaceData;
	for (int i=0; i<mesh->Faces(); ++i)
	{
		FEFace& f = mesh->Face(i);
		FACEDATA& d = state.m_FACE[i];
		d.m_ntag = 0;
		ELEMDATA& e = state.m_ELEM[f.m_elem[0]];
		if (e.m_state & StatusFlags::ACTIVE)
		{
			d.m_ntag = 1;

			switch (mesh->Element(f.m_elem[0]).Type())
			{
			case FE_TET4:
			case FE_TET5:
			{
					const int* fn = FT_TET[f.m_elem[1]];
					fd.value(i, 0) = elemData.value(f.m_elem[0], fn[0]);
					fd.value(i, 1) = elemData.value(f.m_elem[0], fn[1]);
					fd.value(i, 2) = elemData.value(f.m_elem[0], fn[2]);
					d.m_val = (fd.value(i, 0) + fd.value(i, 1) + fd.value(i, 2)) / 3.f;
				}
				break;
			case FE_PENTA6:
				{
					const int* fn = FT_PENTA[f.m_elem[1]];
                    switch (f.m_ntype) {
                        case FACE_TRI3:
                            fd.value(i, 0) = elemData.value(f.m_elem[0], fn[0]);
                            fd.value(i, 1) = elemData.value(f.m_elem[0], fn[1]);
                            fd.value(i, 2) = elemData.value(f.m_elem[0], fn[2]);
                            d.m_val = (fd.value(i, 0) + fd.value(i, 1) + fd.value(i, 2)) / 3.f;
                            break;
                        case FACE_QUAD4:
                            fd.value(i, 0) = elemData.value(f.m_elem[0], fn[0]);
                            fd.value(i, 1) = elemData.value(f.m_elem[0], fn[1]);
                            fd.value(i, 2) = elemData.value(f.m_elem[0], fn[2]);
                            fd.value(i, 3) = elemData.value(f.m_elem[0], fn[3]);
                            d.m_val = (fd.value(i, 0) + fd.value(i, 1) + fd.value(i, 2) + fd.value(i, 3)) * 0.25f;
                            break;
                        default:
                            break;
                    }
				}
				break;
            case FE_PENTA15:
                {
                    const int* fn = FT_PENTA[f.m_elem[1]];
                    switch (f.m_ntype) {
                        case FACE_TRI6:
                            fd.value(i, 0) = elemData.value(f.m_elem[0], fn[0]);
                            fd.value(i, 1) = elemData.value(f.m_elem[0], fn[1]);
                            fd.value(i, 2) = elemData.value(f.m_elem[0], fn[2]);
                            fd.value(i, 3) = elemData.value(f.m_elem[0], fn[3]);
                            fd.value(i, 4) = elemData.value(f.m_elem[0], fn[4]);
                            fd.value(i, 5) = elemData.value(f.m_elem[0], fn[5]);
                            d.m_val = (fd.value(i, 0) + fd.value(i, 1) + fd.value(i, 2) + fd.value(i, 3) + fd.value(i, 4) + fd.value(i, 5)) / 6.f;
                            break;
                        case FACE_QUAD8:
                            fd.value(i, 0) = elemData.value(f.m_elem[0], fn[0]);
                            fd.value(i, 1) = elemData.value(f.m_elem[0], fn[1]);
                            fd.value(i, 2) = elemData.value(f.m_elem[0], fn[2]);
                            fd.value(i, 3) = elemData.value(f.m_elem[0], fn[3]);
                            fd.value(i, 4) = elemData.value(f.m_elem[0], fn[4]);
                            fd.value(i, 5) = elemData.value(f.m_elem[0], fn[5]);
                            fd.value(i, 6) = elemData.value(f.m_elem[0], fn[6]);
                            fd.value(i, 7) = elemData.value(f.m_elem[0], fn[7]);
                            d.m_val = (fd.value(i, 0) + fd.value(i, 1) + fd.value(i, 2) + fd.value(i, 3) + fd.value(i, 4) + fd.value(i, 5) + fd.value(i, 6) + fd.value(i, 7)) * 0.125f;
                            break;
                        default:
                            break;
                    }
                }
                break;
            case FE_HEX8:
                {
                    const int* fn = FT_HEX[f.m_elem[1]];
					fd.value(i, 0) = elemData.value(f.m_elem[0], fn[0]);
					fd.value(i, 1) = elemData.value(f.m_elem[0], fn[1]);
					fd.value(i, 2) = elemData.value(f.m_elem[0], fn[2]);
					fd.value(i, 3) = elemData.value(f.m_elem[0], fn[3]);
					d.m_val = 0.25f*(fd.value(i, 0) + fd.value(i, 1) + fd.value(i, 2) + fd.value(i, 3));
				}
				break;
			case FE_TET10:
				{
					const int* fn = FT_TET10[f.m_elem[1]];
					fd.value(i, 0) = elemData.value(f.m_elem[0], fn[0]);
					fd.value(i, 1) = elemData.value(f.m_elem[0], fn[1]);
					fd.value(i, 2) = elemData.value(f.m_elem[0], fn[2]);
					fd.value(i, 3) = elemData.value(f.m_elem[0], fn[3]);
					fd.value(i, 4) = elemData.value(f.m_elem[0], fn[4]);
					fd.value(i, 5) = elemData.value(f.m_elem[0], fn[5]);
					d.m_val = (fd.value(i, 0) + fd.value(i, 1) + fd.value(i, 2) + fd.value(i, 3) + fd.value(i, 4) + fd.value(i, 5)) / 6.f;
				}
				break;
			case FE_TET15:
				{
					const int* fn = FT_TET15[f.m_elem[1]];
					fd.value(i, 0) = elemData.value(f.m_elem[0], fn[0]);
					fd.value(i, 1) = elemData.value(f.m_elem[0], fn[1]);
					fd.value(i, 2) = elemData.value(f.m_elem[0], fn[2]);
					fd.value(i, 3) = elemData.value(f.m_elem[0], fn[3]);
					fd.value(i, 4) = elemData.value(f.m_elem[0], fn[4]);
					fd.value(i, 5) = elemData.value(f.m_elem[0], fn[5]);
					fd.value(i, 6) = elemData.value(f.m_elem[0], fn[6]);
					d.m_val = (fd.value(i, 0) + fd.value(i, 1) + fd.value(i, 2) + fd.value(i, 3) + fd.value(i, 4) + fd.value(i, 5) + fd.value(i, 6)) / 7.f;
				}
				break;
			case FE_TET20:
				{
					const int* fn = FT_TET20[f.m_elem[1]];
					fd.value(i, 0) = elemData.value(f.m_elem[0], fn[0]);
					fd.value(i, 1) = elemData.value(f.m_elem[0], fn[1]);
					fd.value(i, 2) = elemData.value(f.m_elem[0], fn[2]);
					fd.value(i, 3) = elemData.value(f.m_elem[0], fn[3]);
					fd.value(i, 4) = elemData.value(f.m_elem[0], fn[4]);
					fd.value(i, 5) = elemData.value(f.m_elem[0], fn[5]);
					fd.value(i, 6) = elemData.value(f.m_elem[0], fn[6]);
					fd.value(i, 7) = elemData.value(f.m_elem[0], fn[7]);
					fd.value(i, 8) = elemData.value(f.m_elem[0], fn[8]);
					fd.value(i, 9) = elemData.value(f.m_elem[0], fn[9]);
					d.m_val = (fd.value(i, 0) + fd.value(i, 1) + fd.value(i, 2) + fd.value(i, 3) + fd.value(i, 4) + fd.value(i, 5) + fd.value(i, 6) + fd.value(i, 7) + fd.value(i, 8) + fd.value(i, 9)) / 10.f;
				}
				break;
			case FE_HEX20:
				{
					const int* fn = FT_HEX20[f.m_elem[1]];
					fd.value(i, 0) = elemData.value(f.m_elem[0], fn[0]);
					fd.value(i, 1) = elemData.value(f.m_elem[0], fn[1]);
					fd.value(i, 2) = elemData.value(f.m_elem[0], fn[2]);
					fd.value(i, 3) = elemData.value(f.m_elem[0], fn[3]);
					fd.value(i, 4) = elemData.value(f.m_elem[0], fn[4]);
					fd.value(i, 5) = elemData.value(f.m_elem[0], fn[5]);
					fd.value(i, 6) = elemData.value(f.m_elem[0], fn[6]);
					fd.value(i, 7) = elemData.value(f.m_elem[0], fn[7]);
					d.m_val = (fd.value(i, 0)+fd.value(i, 1)+fd.value(i, 2)+fd.value(i, 3)+fd.value(i, 4)+fd.value(i, 5)+fd.value(i, 6)+fd.value(i, 7))*0.125f;
				}
				break;
			case FE_HEX27:
				{
					const int* fn = FT_HEX27[f.m_elem[1]];
					fd.value(i, 0) = elemData.value(f.m_elem[0], fn[0]);
					fd.value(i, 1) = elemData.value(f.m_elem[0], fn[1]);
					fd.value(i, 2) = elemData.value(f.m_elem[0], fn[2]);
					fd.value(i, 3) = elemData.value(f.m_elem[0], fn[3]);
					fd.value(i, 4) = elemData.value(f.m_elem[0], fn[4]);
					fd.value(i, 5) = elemData.value(f.m_elem[0], fn[5]);
					fd.value(i, 6) = elemData.value(f.m_elem[0], fn[6]);
					fd.value(i, 7) = elemData.value(f.m_elem[0], fn[7]);
					d.m_val = (fd.value(i, 0)+fd.value(i, 1)+fd.value(i, 2)+fd.value(i, 3)+fd.value(i, 4)+fd.value(i, 5)+fd.value(i, 6)+fd.value(i, 7))*0.125f;
				}
				break;
			case FE_TRI3:
				{
					fd.value(i, 0) = elemData.value(f.m_elem[0], 0);
					fd.value(i, 1) = elemData.value(f.m_elem[0], 1);
					fd.value(i, 2) = elemData.value(f.m_elem[0], 2);
					d.m_val = (fd.value(i, 0)+fd.value(i, 1)+fd.value(i, 2))/3.0f;
				}
				break;
			case FE_QUAD4:
				{
					fd.value(i, 0) = elemData.value(f.m_elem[0], 0);
					fd.value(i, 1) = elemData.value(f.m_elem[0], 1);
					fd.value(i, 2) = elemData.value(f.m_elem[0], 2);
					fd.value(i, 3) = elemData.value(f.m_elem[0], 3);
					d.m_val = (fd.value(i, 0)+fd.value(i, 1)+fd.value(i, 2)+fd.value(i, 3))*0.25f;
				}
				break;
            case FE_QUAD8:
                {
                    fd.value(i, 0) = elemData.value(f.m_elem[0], 0);
                    fd.value(i, 1) = elemData.value(f.m_elem[0], 1);
                    fd.value(i, 2) = elemData.value(f.m_elem[0], 2);
                    fd.value(i, 3) = elemData.value(f.m_elem[0], 3);
                    fd.value(i, 4) = elemData.value(f.m_elem[0], 4);
                    fd.value(i, 5) = elemData.value(f.m_elem[0], 5);
                    fd.value(i, 6) = elemData.value(f.m_elem[0], 6);
                    fd.value(i, 7) = elemData.value(f.m_elem[0], 7);
 					d.m_val = (fd.value(i, 0)+fd.value(i, 1)+fd.value(i, 2)+fd.value(i, 3)+fd.value(i, 4)+fd.value(i, 5)+fd.value(i, 6)+fd.value(i, 7))*0.125f;
                }
                break;
            case FE_QUAD9:
                {
                    fd.value(i, 0) = elemData.value(f.m_elem[0], 0);
                    fd.value(i, 1) = elemData.value(f.m_elem[0], 1);
                    fd.value(i, 2) = elemData.value(f.m_elem[0], 2);
                    fd.value(i, 3) = elemData.value(f.m_elem[0], 3);
                    fd.value(i, 4) = elemData.value(f.m_elem[0], 4);
                    fd.value(i, 5) = elemData.value(f.m_elem[0], 5);
                    fd.value(i, 6) = elemData.value(f.m_elem[0], 6);
                    fd.value(i, 7) = elemData.value(f.m_elem[0], 7);
                    fd.value(i, 8) = elemData.value(f.m_elem[0], 8);
                    d.m_val = (fd.value(i, 0)+fd.value(i, 1)+fd.value(i, 2)+fd.value(i, 3)+fd.value(i, 4)+fd.value(i, 5)+fd.value(i, 6)+fd.value(i, 7)+fd.value(i, 8))/9.0f;
                }
                break;
            case FE_TRI6:
                {
                    fd.value(i, 0) = elemData.value(f.m_elem[0], 0);
                    fd.value(i, 1) = elemData.value(f.m_elem[0], 1);
                    fd.value(i, 2) = elemData.value(f.m_elem[0], 2);
                    fd.value(i, 3) = elemData.value(f.m_elem[0], 3);
                    fd.value(i, 4) = elemData.value(f.m_elem[0], 4);
                    fd.value(i, 5) = elemData.value(f.m_elem[0], 5);
                    d.m_val = (fd.value(i, 0)+fd.value(i, 1)+fd.value(i, 2)+fd.value(i, 3)+fd.value(i, 4)+fd.value(i, 5))/6.0f;
                }
				break;
			default:
				assert(false);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Get the field value of node n at time ntime
void FEModel::EvaluateNode(int n, int ntime, int nfield, NODEDATA& d)
{
	FEState& state = *GetState(ntime);
	FEMeshBase* mesh = state.GetFEMesh();
	m_ntime = ntime;

	// the return value
	d.m_val = 0.f;
	d.m_ntag = 1;

	if (state.m_Data.size() == 0) 
	{
		d.m_val = 0.0f;
		return;
	}

	if (IS_NODE_FIELD(nfield))
	{
		// get the data ID
		int ndata = FIELD_CODE(nfield);
		assert((ndata >= 0) && (ndata < state.m_Data.size()));

		// get the component
		int ncomp = FIELD_COMP(nfield);

		FEMeshData& rd = state.m_Data[ndata];
		Data_Format fmt = rd.GetFormat();
		assert(fmt == DATA_ITEM);

		switch (rd.GetType())
		{
		case DATA_FLOAT: 
			{
				FENodeData_T<float>& df = dynamic_cast<FENodeData_T<float>&>(rd);
				df.eval(n, &d.m_val);
			}
			break;
		case DATA_VEC3F:
			{
				FENodeData_T<vec3f>& dv = dynamic_cast<FENodeData_T<vec3f>&>(rd);
				vec3f v;
				dv.eval(n,&v);
				d.m_val = component(v, ncomp);
			}
			break;
		case DATA_MAT3F:
			{
				FENodeData_T<mat3f>& dm = dynamic_cast<FENodeData_T<mat3f>&>(rd);
				mat3f m;
				dm.eval(n, &m);
				d.m_val = component(m, ncomp);
			}
			break;
		case DATA_MAT3D:
			{
				FENodeData_T<Mat3d>& dm = dynamic_cast<FENodeData_T<Mat3d>&>(rd);
				Mat3d m;
				dm.eval(n, &m);
				d.m_val = component(m, ncomp);
			}
			break;
		case DATA_MAT3FS:
			{
				FENodeData_T<mat3fs>& dm = dynamic_cast<FENodeData_T<mat3fs>&>(rd);
				mat3fs m;
				dm.eval(n, &m);
				d.m_val = component(m, ncomp);
			}
			break;
		case DATA_MAT3FD:
			{
				FENodeData_T<mat3fd>& dm = dynamic_cast<FENodeData_T<mat3fd>&>(rd);
				mat3fd m;
				dm.eval(n, &m);
				d.m_val = component(m, ncomp);
			}
			break;
        case DATA_TENS4FS:
			{
				FENodeData_T<tens4fs>& dm = dynamic_cast<FENodeData_T<tens4fs>&>(rd);
				tens4fs m;
				dm.eval(n, &m);
				d.m_val = component(m, ncomp);
			}
            break;
		case DATA_ARRAY:
			{
				FENodeArrayData& dm = dynamic_cast<FENodeArrayData&>(rd);
				d.m_val = dm.eval(n, ncomp); 
			}
			break;
		}
	}
	else if (IS_FACE_FIELD(nfield))
	{
		// we take the average of the adjacent face values
		vector<NodeFaceRef>& nfl = mesh->NodeFaceList(n);
		if (!nfl.empty())
		{
			int nf = (int)nfl.size(), n = 0;
			float data[FEFace::MAX_NODES], val;
			for (int i=0; i<nf; ++i)
			{
				if (EvaluateFace(nfl[i].first, ntime, nfield, data, val))
				{
					d.m_val += data[nfl[i].second];
					++n;
				}
			}
			if (n>0) d.m_val /= (float) n;
		}
	}
	else if (IS_ELEM_FIELD(nfield))
	{
		// we take the average of the elements that contain this element
		vector<NodeElemRef>& nel = mesh->NodeElemList(n);
		float data[FEGenericElement::MAX_NODES] = {0.f}, val;
		int ne = (int)nel.size(), n = 0;
		if (!nel.empty())
		{
			for (int i=0; i<ne; ++i)
			{
				if (EvaluateElement(nel[i].first, ntime, nfield, data, val))
				{
					d.m_val += data[nel[i].second];
					++n;
				}
			}
			if (n != 0) d.m_val /= (float) n;
		}
	}
	else assert(false);
}

//-----------------------------------------------------------------------------
void FEModel::EvaluateNode(const vec3f& p, int ntime, int nfield, NODEDATA& d)
{
	// find the element in which this point lies
	FEMeshBase& mesh = *GetState(ntime)->GetFEMesh();
	int elid = -1;
	double r[3];
	if (FindElementRef(mesh, p, elid, r) == false)
	{
		d.m_ntag = 0; 
		return;
	}
	FEElement& el = mesh.Element(elid);

	// evaluate the nodal values
	float v[FEGenericElement::MAX_NODES] = { 0.f }, val = 0.f;
	EvaluateElement(elid, ntime, nfield, v, val);

	d.m_ntag = 1;
	d.m_val = el.eval(v, r[0], r[1], r[2]);
}

//-----------------------------------------------------------------------------
// Calculate field value of edge n at time ntime
void FEModel::EvaluateEdge(int n, int ntime, int nfield, EDGEDATA& d)
{
	d.m_val = 0.f;
	d.m_ntag = 0;
	d.m_nv[0] = d.m_nv[1] = d.m_nv[2] = 0.f;
}

//-----------------------------------------------------------------------------
// Calculate field value of face n at time ntime
bool FEModel::EvaluateFace(int n, int ntime, int nfield, float* data, float& val)
{
	m_ntime = ntime;

	// get the face
	FEState& state = *GetState(ntime);
	FEMeshBase* mesh = state.GetFEMesh();

	FEFace& f = mesh->Face(n);
	int nf = f.Nodes();

	// the return value
	for (int i=0; i<nf; ++i) data[i] = 0.f;
	val = 0;
	int ntag = 0;

	// get the state
	FEState& s = *m_State[ntime];


	if (IS_FACE_FIELD(nfield))
	{
		// get the data ID
		int ndata = FIELD_CODE(nfield);
		assert ((ndata >= 0) && (ndata < s.m_Data.size()));

		// get the component
		int ncomp = FIELD_COMP(nfield);

		FEMeshData& rd = s.m_Data[ndata];
		Data_Format fmt = rd.GetFormat();

		switch (rd.GetType())
		{
		case DATA_FLOAT: 
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData_T<float,DATA_NODE>& df = dynamic_cast<FEFaceData_T<float,DATA_NODE>&>(rd);
					if (df.active(n))
					{
						df.eval(n, data);
						val = 0.f;
						for (int i=0; i<nf; ++i) val += data[i];
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData_T<float,DATA_ITEM>& df = dynamic_cast<FEFaceData_T<float,DATA_ITEM>&>(rd);
					if (df.active(n))
					{
						df.eval(n, &val);
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEFaceData_T<float,DATA_COMP>& df = dynamic_cast<FEFaceData_T<float,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						df.eval(n, data);
						val = 0.f;
						for (int i=0; i<nf; ++i) val += data[i];
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData_T<float,DATA_REGION>& df = dynamic_cast<FEFaceData_T<float,DATA_REGION>&>(rd);
					if (df.active(n))
					{
						df.eval(n, &val);
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
			}
			break;
		case DATA_VEC3F:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData_T<vec3f,DATA_NODE>& df = dynamic_cast<FEFaceData_T<vec3f,DATA_NODE>&>(rd);
					if (df.active(n))
					{
						vec3f v[FEFace::MAX_NODES];
						df.eval(n, v);
						int nf = f.Nodes();
						val = 0.f;
						for (int i=0; i<nf; ++i)
						{
							data[i] = component(v[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData_T<vec3f,DATA_ITEM>& dv = dynamic_cast<FEFaceData_T<vec3f,DATA_ITEM>&>(rd);
					if (dv.active(n))
					{
						vec3f v;
						dv.eval(n, &v);
						val = component(v, ncomp);
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEFaceData_T<vec3f,DATA_COMP>& df = dynamic_cast<FEFaceData_T<vec3f,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						vec3f v[FEFace::MAX_NODES];
						df.eval(n, v);
						int nf = f.Nodes();
						val = 0.f;
						for (int i=0; i<nf; ++i) 
						{
							data[i] = component(v[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData_T<vec3f,DATA_REGION>& dv = dynamic_cast<FEFaceData_T<vec3f,DATA_REGION>&>(rd);
					if (dv.active(n))
					{
						vec3f v;
						dv.eval(n, &v);
						val = component(v, ncomp);
						int nf = f.Nodes();
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
			}
			break;
		case DATA_MAT3F:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData_T<mat3f,DATA_NODE>& df = dynamic_cast<FEFaceData_T<mat3f,DATA_NODE>&>(rd);
					if (df.active(n))
					{
						mat3f m[FEFace::MAX_NODES];
						df.eval(n, m);

						val = 0.f;
						for (int i=0; i<nf; ++i) 
						{
							data[i] = component(m[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;

						ntag = 1;
					}
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData_T<mat3f,DATA_ITEM>& dv = dynamic_cast<FEFaceData_T<mat3f,DATA_ITEM>&>(rd);
					if (dv.active(n))
					{
						mat3f m;
						dv.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEFaceData_T<mat3f,DATA_COMP>& df = dynamic_cast<FEFaceData_T<mat3f,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						mat3f m[FEFace::MAX_NODES];
						df.eval(n, m);
						val = 0.f;
						for (int i=0; i<nf; ++i) 
						{
							data[i] = component(m[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData_T<mat3f,DATA_REGION>& dv = dynamic_cast<FEFaceData_T<mat3f,DATA_REGION>&>(rd);
					if (dv.active(n))
					{
						mat3f m;
						dv.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
			}
			break;
		case DATA_MAT3D:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData_T<Mat3d,DATA_NODE>& df = dynamic_cast<FEFaceData_T<Mat3d,DATA_NODE>&>(rd);
					if (df.active(n))
					{
						Mat3d m[FEFace::MAX_NODES];
						df.eval(n, m);
						int nf = f.Nodes();
						val = 0.f;
						for (int i=0; i<nf; ++i)
						{
							data[i] = component(m[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData_T<Mat3d,DATA_ITEM>& dv = dynamic_cast<FEFaceData_T<Mat3d,DATA_ITEM>&>(rd);
					if (dv.active(n))
					{
						Mat3d m;
						dv.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEFaceData_T<Mat3d,DATA_COMP>& df = dynamic_cast<FEFaceData_T<Mat3d,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						Mat3d m[FEFace::MAX_NODES];
						df.eval(n, m);
						val = 0.f;
						for (int i=0; i<nf; ++i)
						{
							data[i] = component(m[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;
						ntag = 1;
					}
				}
			}
			break;
		case DATA_MAT3FS:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData_T<mat3fs,DATA_NODE>& df = dynamic_cast<FEFaceData_T<mat3fs,DATA_NODE>&>(rd);
					if (df.active(n))
					{
						mat3fs m[FEFace::MAX_NODES];
						df.eval(n, m);
						val = 0.f;
						for (int i=0; i<nf; ++i)
						{
							data[i] = component(m[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData_T<mat3fs,DATA_ITEM>& dv = dynamic_cast<FEFaceData_T<mat3fs,DATA_ITEM>&>(rd);
					if (dv.active(n))
					{
						mat3fs m;
						dv.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEFaceData_T<mat3fs,DATA_COMP>& df = dynamic_cast<FEFaceData_T<mat3fs,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						mat3fs m[FEFace::MAX_NODES];
						df.eval(n, m);
						val = 0.f;
						for (int i=0; i<nf; ++i)
						{
							data[i] = component(m[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData_T<mat3fs,DATA_REGION>& dv = dynamic_cast<FEFaceData_T<mat3fs,DATA_REGION>&>(rd);
					if (dv.active(n))
					{
						mat3fs m;
						dv.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
			}
			break;
		case DATA_MAT3FD:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData_T<mat3fd,DATA_NODE>& df = dynamic_cast<FEFaceData_T<mat3fd,DATA_NODE>&>(rd);
					if (df.active(n))
					{
						mat3fd m[FEFace::MAX_NODES];
						df.eval(n, m);
						val = 0.f;
						for (int i=0; i<nf; ++i)
						{
							data[i] = component(m[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData_T<mat3fd,DATA_ITEM>& dv = dynamic_cast<FEFaceData_T<mat3fd,DATA_ITEM>&>(rd);
					if (dv.active(n))
					{
						mat3fd m;
						dv.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEFaceData_T<mat3fd,DATA_COMP>& df = dynamic_cast<FEFaceData_T<mat3fd,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						mat3fd m[FEFace::MAX_NODES];
						df.eval(n, m);
						val = 0.f;
						for (int i=0; i<nf; ++i)
						{
							data[i] = component(m[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData_T<mat3fd,DATA_REGION>& dv = dynamic_cast<FEFaceData_T<mat3fd,DATA_REGION>&>(rd);
					if (dv.active(n))
					{
						mat3fd m;
						dv.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
			}
			break;
        case DATA_TENS4FS:
			{
				if (fmt == DATA_NODE)
				{
					FEFaceData_T<tens4fs,DATA_NODE>& df = dynamic_cast<FEFaceData_T<tens4fs,DATA_NODE>&>(rd);
					if (df.active(n))
					{
						tens4fs m[FEFace::MAX_NODES];
						df.eval(n, m);
						int nf = f.Nodes();
						val = 0.f;
						for (int i=0; i<nf; ++i)
						{
							data[i] = component(m[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_ITEM)
				{
					FEFaceData_T<tens4fs,DATA_ITEM>& dv = dynamic_cast<FEFaceData_T<tens4fs,DATA_ITEM>&>(rd);
					if (dv.active(n))
					{
						tens4fs m;
						dv.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<nf; ++i) data[0] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEFaceData_T<tens4fs,DATA_COMP>& df = dynamic_cast<FEFaceData_T<tens4fs,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						tens4fs m[FEFace::MAX_NODES];
						df.eval(n, m);
						val = 0.f;
						for (int i=0; i<nf; ++i) 
						{
							data[i] = component(m[i], ncomp);
							val += data[i];
						}
						val /= (float) nf;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEFaceData_T<tens4fs,DATA_REGION>& dv = dynamic_cast<FEFaceData_T<tens4fs,DATA_REGION>&>(rd);
					if (dv.active(n))
					{
						tens4fs m;
						dv.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<nf; ++i) data[i] = val;
						ntag = 1;
					}
				}
			}
            break;
		}
	}
	else if (IS_NODE_FIELD(nfield))
	{
		int N = f.Nodes();
		NODEDATA n;
		for (int i=0; i<N; ++i) 
		{ 
			EvaluateNode(f.node[i], ntime, nfield, n); 
			data[i] = n.m_val;
			val += data[i]; 
		}
		val /= (float) N;
		ntag = 1;
	}
	else if (IS_ELEM_FIELD(nfield))
	{
		assert((f.m_elem[0] >= 0) && (f.m_elem[0] < mesh->Elements()));
		float edata[FEGenericElement::MAX_NODES] = {0.f};
		EvaluateElement(f.m_elem[0], ntime, nfield, edata, val);
		ntag = 1;
		switch (mesh->Element(f.m_elem[0]).Type())
		{
		case FE_TET4:
		case FE_TET5:
		{
				const int* fn = FT_TET[f.m_elem[1]];
				data[0] = edata[fn[0]];
				data[1] = edata[fn[1]];
				data[2] = edata[fn[2]];
				val = (data[0] + data[1] + data[2]) / 3.f;
			}
			break;
        case FE_PENTA6:
            {
                const int* fn = FT_PENTA[f.m_elem[1]];
                switch (f.m_ntype) {
                    case FACE_TRI3:
                        data[0] = edata[fn[0]];
                        data[1] = edata[fn[1]];
                        data[2] = edata[fn[2]];
                        val = (data[0] + data[1] + data[2]) / 3.f;
                        break;
                    case FACE_QUAD4:
                        data[0] = edata[fn[0]];
                        data[1] = edata[fn[1]];
                        data[2] = edata[fn[2]];
                        data[3] = edata[fn[3]];
                        val = 0.25f*(data[0] + data[1] + data[2] + data[3]);
                        break;
                    default:
                        break;
                }
            }
            break;
        case FE_PENTA15:
            {
                const int* fn = FT_PENTA[f.m_elem[1]];
                switch (f.m_ntype) {
                    case FACE_TRI6:
                        data[0] = edata[fn[0]];
                        data[1] = edata[fn[1]];
                        data[2] = edata[fn[2]];
                        data[3] = edata[fn[3]];
                        data[4] = edata[fn[4]];
                        data[5] = edata[fn[5]];
                        val = (data[0] + data[1] + data[2] + data[3] + data[4] + data[5]) / 6.f;
                        break;
                    case FACE_QUAD8:
                        data[0] = edata[fn[0]];
                        data[1] = edata[fn[1]];
                        data[2] = edata[fn[2]];
                        data[3] = edata[fn[3]];
                        data[4] = edata[fn[4]];
                        data[5] = edata[fn[5]];
                        data[6] = edata[fn[6]];
                        data[7] = edata[fn[7]];
                        val = (data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7]) / 8.f;
                        break;
                    default:
                        break;
                }
            }
            break;
            case FE_HEX8:
            {
                const int* fn = FT_HEX[f.m_elem[1]];
                data[0] = edata[fn[0]];
                data[1] = edata[fn[1]];
                data[2] = edata[fn[2]];
                data[3] = edata[fn[3]];
                val = 0.25f*(data[0] + data[1] + data[2] + data[3]);
			}
			break;
		case FE_TET10:
			{
				const int* fn = FT_TET10[f.m_elem[1]];
                data[0] = edata[fn[0]];
                data[1] = edata[fn[1]];
                data[2] = edata[fn[2]];
                data[3] = edata[fn[3]];
                data[4] = edata[fn[4]];
                data[5] = edata[fn[5]];
				val = (data[0] + data[1] + data[2] + data[3] + data[4] + data[5]) / 6.f;
			}
			break;
		case FE_TET15:
			{
				const int* fn = FT_TET15[f.m_elem[1]];
                data[0] = edata[fn[0]];
                data[1] = edata[fn[1]];
                data[2] = edata[fn[2]];
                data[3] = edata[fn[3]];
                data[4] = edata[fn[4]];
                data[5] = edata[fn[5]];
                data[6] = edata[fn[6]];
				val = (data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6]) / 7.f;
			}
			break;
		case FE_TET20:
			{
				const int* fn = FT_TET20[f.m_elem[1]];
                data[0] = edata[fn[0]];
                data[1] = edata[fn[1]];
                data[2] = edata[fn[2]];
                data[3] = edata[fn[3]];
                data[4] = edata[fn[4]];
                data[5] = edata[fn[5]];
                data[6] = edata[fn[6]];
				data[7] = edata[fn[8]];
				data[8] = edata[fn[9]];
				data[9] = edata[fn[0]];
				val = (data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7] + data[8] + data[9]) / 10.f;
			}
			break;
		case FE_HEX20:
			{
				const int* fn = FT_HEX20[f.m_elem[1]];
                data[0] = edata[fn[0]];
                data[1] = edata[fn[1]];
                data[2] = edata[fn[2]];
                data[3] = edata[fn[3]];
                data[4] = edata[fn[4]];
                data[5] = edata[fn[5]];
                data[6] = edata[fn[6]];
                data[7] = edata[fn[7]];
				val = (data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7])*0.125f;
			}
			break;
		case FE_HEX27:
			{
				const int* fn = FT_HEX27[f.m_elem[1]];
                data[0] = edata[fn[0]];
                data[1] = edata[fn[1]];
                data[2] = edata[fn[2]];
                data[3] = edata[fn[3]];
                data[4] = edata[fn[4]];
                data[5] = edata[fn[5]];
                data[6] = edata[fn[6]];
                data[7] = edata[fn[7]];
                data[8] = edata[fn[8]];
				val = (data[0] + data[1] + data[2] + data[3] + data[4] + data[5] + data[6] + data[7] + data[8]) / 9.f;
			}
			break;
		case FE_QUAD4:
		case FE_QUAD8:
		case FE_TRI3:
		case FE_TRI6:
			{
				int nn = f.Nodes();
				val = 0.0;
				for (int i=0; i<nn; ++i)
				{
					data[i] = edata[i];
					val += data[i];
				}
				val /= (nn);
			}
			break;
		default:
			assert(false);
		}
	}
	else
	{
		assert(false);
	}
	return (ntag == 1);
}

//-----------------------------------------------------------------------------
bool FEModel::EvaluateElement(int n, int ntime, int nfield, float* data, float& val)
{
	FEState& state = *GetState(ntime);
	FEMeshBase* mesh = state.GetFEMesh();
	m_ntime = ntime;

	// get the element
	FEElement& el = mesh->Element(n);
	int ne = el.Nodes();

	// the return value
	val = 0.f;
	for (int i=0; i<ne; ++i) data[i] = 0.f;
	int ntag = 0;

	if (IS_ELEM_FIELD(nfield))
	{
		// get the data ID
		int ndata = FIELD_CODE(nfield);
		assert((ndata >= 0) && (ndata < state.m_Data.size()));

		// get the component
		int ncomp = FIELD_COMP(nfield);

		FEMeshData& rd = state.m_Data[ndata];
		Data_Format fmt = rd.GetFormat(); 

		switch (rd.GetType())
		{
		case DATA_FLOAT: 
			{
				if (fmt == DATA_NODE)
				{
					FEElemData_T<float,DATA_NODE>& df = dynamic_cast<FEElemData_T<float,DATA_NODE>&>(rd);
					if (df.active(n))
					{
						df.eval(n, data);
						val = 0.f;
						for (int j=0; j<ne; ++j) val += data[j];
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_ITEM)
				{
					FEElemData_T<float,DATA_ITEM>& df = dynamic_cast<FEElemData_T<float,DATA_ITEM>&>(rd);
					if (df.active(n))
					{
						df.eval(n, &val);
						for (int j=0; j<ne; ++j) data[j] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEElemData_T<float,DATA_COMP>& df = dynamic_cast<FEElemData_T<float,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						df.eval(n, data);
						val = 0;
						for (int j=0; j<ne; ++j) val += data[j];
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEElemData_T<float,DATA_REGION>& df = dynamic_cast<FEElemData_T<float,DATA_REGION>&>(rd);
					if (df.active(n))
					{
						df.eval(n, &val);
						for (int j=0; j<ne; ++j) data[j] = val;
						ntag = 1;
					}
				}
			}
			break;
		case DATA_VEC3F:
			{
				if (fmt == DATA_ITEM)
				{
					FEElemData_T<vec3f,DATA_ITEM>& dv = dynamic_cast<FEElemData_T<vec3f,DATA_ITEM>&>(rd);
					if (dv.active(n))
					{
						vec3f v;
						dv.eval(n, &v);
						val = component(v, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEElemData_T<vec3f,DATA_COMP>& df = dynamic_cast<FEElemData_T<vec3f,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						vec3f v[FEGenericElement::MAX_NODES];
						df.eval(n, v);
						val = 0;
						for (int j=0; j<ne; ++j) 
						{
							data[j] = component(v[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_NODE)
				{
					FEElemData_T<vec3f,DATA_NODE>& dm = dynamic_cast<FEElemData_T<vec3f,DATA_NODE>&>(rd);
					if (dm.active(n))
					{
						vec3f v[FEGenericElement::MAX_NODES];
						dm.eval(n, v);
						val = 0;
						for (int j=0; j<ne; ++j)
						{
							data[j] = component(v[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEElemData_T<vec3f,DATA_REGION>& dv = dynamic_cast<FEElemData_T<vec3f,DATA_REGION>&>(rd);
					if (dv.active(n))
					{
						vec3f v;
						dv.eval(n, &v);
						val = component(v, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else
				{
					assert(false);
				}
			}
			break;
		case DATA_MAT3D:
			{
				if (fmt == DATA_ITEM)
				{
					FEElemData_T<Mat3d,DATA_ITEM>& dm = dynamic_cast<FEElemData_T<Mat3d,DATA_ITEM>&>(rd);
					if (dm.active(n))
					{
						Mat3d m;
						dm.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_NODE)
				{
					FEElemData_T<Mat3d,DATA_NODE>& dm = dynamic_cast<FEElemData_T<Mat3d,DATA_NODE>&>(rd);
					if (dm.active(n))
					{
						Mat3d m[FEGenericElement::MAX_NODES];
						dm.eval(n, m);
						val = 0;
						for (int j=0; j<ne; ++j)
						{
							data[j] = component(m[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEElemData_T<Mat3d,DATA_COMP>& df = dynamic_cast<FEElemData_T<Mat3d,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						Mat3d v[FEGenericElement::MAX_NODES];
						df.eval(n, v);
						val = 0;
						for (int j=0; j<ne; ++j) 
						{
							data[j] = component(v[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEElemData_T<Mat3d,DATA_REGION>& dm = dynamic_cast<FEElemData_T<Mat3d,DATA_REGION>&>(rd);
					if (dm.active(n))
					{
						Mat3d m;
						dm.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else
				{
					assert(false);
				}
			}
			break;
		case DATA_MAT3F:
			{
				if (fmt == DATA_ITEM)
				{
					FEElemData_T<mat3f,DATA_ITEM>& dm = dynamic_cast<FEElemData_T<mat3f,DATA_ITEM>&>(rd);
					if (dm.active(n))
					{
						mat3f m;
						dm.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_NODE)
				{
					FEElemData_T<mat3f,DATA_NODE>& dm = dynamic_cast<FEElemData_T<mat3f,DATA_NODE>&>(rd);
					if (dm.active(n))
					{
						mat3f m[FEGenericElement::MAX_NODES];
						dm.eval(n, m);
						val = 0;
						for (int j=0; j<ne; ++j)
						{
							data[j] = component(m[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEElemData_T<mat3f,DATA_COMP>& df = dynamic_cast<FEElemData_T<mat3f,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						mat3f v[FEGenericElement::MAX_NODES];
						df.eval(n, v);
						val = 0;
						for (int j=0; j<ne; ++j) 
						{
							data[j] = component(v[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEElemData_T<mat3f,DATA_REGION>& dm = dynamic_cast<FEElemData_T<mat3f,DATA_REGION>&>(rd);
					if (dm.active(n))
					{
						mat3f m;
						dm.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else
				{
					assert(false);
				}
			}
			break;
		case DATA_MAT3FS:
			{
				if (fmt == DATA_ITEM)
				{
					FEElemData_T<mat3fs,DATA_ITEM>& dm = dynamic_cast<FEElemData_T<mat3fs,DATA_ITEM>&>(rd);
					if (dm.active(n))
					{
						mat3fs m;
						dm.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_NODE)
				{
					FEElemData_T<mat3fs,DATA_NODE>& dm = dynamic_cast<FEElemData_T<mat3fs,DATA_NODE>&>(rd);
					if (dm.active(n))
					{
						mat3fs m[FEGenericElement::MAX_NODES];
						dm.eval(n, m);
						val = 0;
						for (int j=0; j<ne; ++j)
						{
							data[j] = component(m[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEElemData_T<mat3fs,DATA_COMP>& df = dynamic_cast<FEElemData_T<mat3fs,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						mat3fs v[FEGenericElement::MAX_NODES];
						df.eval(n, v);
						val = 0;
						for (int j=0; j<ne; ++j) 
						{
							data[j] = component(v[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEElemData_T<mat3fs,DATA_REGION>& dm = dynamic_cast<FEElemData_T<mat3fs,DATA_REGION>&>(rd);
					if (dm.active(n))
					{
						mat3fs m;
						dm.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else
				{
					assert(false);
				}
			}
			break;
		case DATA_MAT3FD:
			{
				if (fmt == DATA_ITEM)
				{
					FEElemData_T<mat3fd,DATA_ITEM>& dm = dynamic_cast<FEElemData_T<mat3fd,DATA_ITEM>&>(rd);
					if (dm.active(n))
					{
						mat3fd m;
						dm.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_NODE)
				{
					FEElemData_T<mat3fd,DATA_NODE>& dm = dynamic_cast<FEElemData_T<mat3fd,DATA_NODE>&>(rd);
					if (dm.active(n))
					{
						mat3fd m[FEGenericElement::MAX_NODES];
						dm.eval(n, m);
						val = 0;
						for (int j=0; j<ne; ++j)
						{
							data[j] = component(m[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEElemData_T<mat3fd,DATA_COMP>& df = dynamic_cast<FEElemData_T<mat3fd,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						mat3fd v[FEGenericElement::MAX_NODES];
						df.eval(n, v);
						val = 0;
						for (int j=0; j<ne; ++j) 
						{
							data[j] = component(v[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEElemData_T<mat3fd,DATA_REGION>& dm = dynamic_cast<FEElemData_T<mat3fd,DATA_REGION>&>(rd);
					if (dm.active(n))
					{
						mat3fd m;
						dm.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else
				{
					assert(false);
				}
			}
			break;		
        case DATA_TENS4FS:
			{
				if (fmt == DATA_ITEM)
				{
					FEElemData_T<tens4fs,DATA_ITEM>& dm = dynamic_cast<FEElemData_T<tens4fs,DATA_ITEM>&>(rd);
					if (dm.active(n))
					{
						tens4fs m;
						dm.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_NODE)
				{
					FEElemData_T<tens4fs,DATA_NODE>& dm = dynamic_cast<FEElemData_T<tens4fs,DATA_NODE>&>(rd);
					if (dm.active(n))
					{
						tens4fs m[FEGenericElement::MAX_NODES];
						dm.eval(n, m);
						val = 0;
						for (int j=0; j<ne; ++j)
						{
							data[j] = component(m[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_COMP)
				{
					FEElemData_T<tens4fs,DATA_COMP>& df = dynamic_cast<FEElemData_T<tens4fs,DATA_COMP>&>(rd);
					if (df.active(n))
					{
						tens4fs v[FEGenericElement::MAX_NODES];
						df.eval(n, v);
						val = 0;
						for (int j=0; j<ne; ++j)
						{
							data[j] = component(v[j], ncomp);
							val += data[j];
						}
						val /= (float) ne;
						ntag = 1;
					}
				}
				else if (fmt == DATA_REGION)
				{
					FEElemData_T<tens4fs,DATA_REGION>& dm = dynamic_cast<FEElemData_T<tens4fs,DATA_REGION>&>(rd);
					if (dm.active(n))
					{
						tens4fs m;
						dm.eval(n, &m);
						val = component(m, ncomp);
						for (int i=0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else
				{
					assert(false);
				}
			}
            break;
		case DATA_ARRAY:
			{
				if (fmt == DATA_ITEM)
				{
					FEElemArrayDataItem& dm = dynamic_cast<FEElemArrayDataItem&>(rd);
					if (dm.active(n))
					{
						val = dm.eval(n, ncomp);
						for (int i = 0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
				else if (fmt == DATA_NODE)
				{
					FEElemArrayDataNode& df = dynamic_cast<FEElemArrayDataNode&>(rd);
					if (df.active(n))
					{
						df.eval(n, ncomp, data);
						val = 0.f;
						for (int j = 0; j<ne; ++j) val += data[j];
						val /= (float)ne;
						ntag = 1;
					}
				}
			}
			break;
		case DATA_ARRAY_VEC3F:
			{
				if (fmt == DATA_ITEM)
				{
					FEElemArrayVec3Data& dm = dynamic_cast<FEElemArrayVec3Data&>(rd);
					if (dm.active(n))
					{
						vec3f v = dm.eval(n, ncomp / 4);
						val = component2(v, ncomp % 4);
						for (int i = 0; i<ne; ++i) data[i] = val;
						ntag = 1;
					}
				}
			}
			break;
		default:
			assert(false);
		}
	}
	else if (IS_NODE_FIELD(nfield))
	{
		// take the average of the nodal values
		NODEDATA n;
		for (int i=0; i<ne; ++i)
		{ 
			EvaluateNode(el.m_node[i], ntime, nfield, n); 
			data[i] = n.m_val;
			val += data[i]; 
		}
		val /= (float) ne;
	}
	else if (IS_FACE_FIELD(nfield))
	{
		// don't do anything; face data is not copied to the elements
	}
	else
	{
		assert(false);
	}

	return (ntag == 1);
}

//-----------------------------------------------------------------------------
// Evaluate vector field at node n at time ntime
vec3f FEModel::EvaluateNodeVector(int n, int ntime, int nvec)
{
	FEState& state = *GetState(ntime);
	FEMeshBase* mesh = state.GetFEMesh();
	m_ntime = ntime;

	vec3f r;

	if (IS_NODE_FIELD(nvec))
	{
		// get the data ID
		int ndata = FIELD_CODE(nvec);
		if ((ndata < 0) || (ndata >= state.m_Data.size())) return vec3f(0.f, 0.f, 0.f);

		// get the component
		int ncomp = FIELD_COMP(nvec);

		FEMeshData& rd = state.m_Data[ndata];
		assert(rd.GetFormat() == DATA_ITEM);

		switch (rd.GetType())
		{
		case DATA_VEC3F:
			{
				FENodeData_T<vec3f>& dv = dynamic_cast<FENodeData_T<vec3f>&>(rd);
				dv.eval(n, &r);
			}
			break;
		case DATA_MAT3FS:
			{
				FENodeData_T<mat3fs>& dm = dynamic_cast<FENodeData_T<mat3fs>&>(rd);
				mat3fs m;
				dm.eval(n, &m);
				r = m.PrincDirection(ncomp+1);
			}
			break;
		default:
			assert(false);
		}
	}
	else if (IS_ELEM_FIELD(nvec))
	{
		// we take the average of the elements that contain this element
		vector<NodeElemRef>& nel = mesh->NodeElemList(n);
		if (!nel.empty())
		{
			int n = 0;
			for (int i=0; i<(int) nel.size(); ++i)
			{
				int iel = nel[i].first;
				FEElement& el = mesh->Element(iel);
				FEMaterial* mat = GetMaterial(el.m_MatID);
				if (mat->benable)
				{
					r += EvaluateElemVector(iel, ntime, nvec);
					n++;
				}
			}
			if (n != 0) r /= (float)n;
		}
	}
	else if (IS_FACE_FIELD(nvec))
	{
		// we take the average of the elements that contain this element
		vector<NodeFaceRef>& nfl = mesh->NodeFaceList(n);
		if (!nfl.empty())
		{
			int n = 0;
			vec3f fv;
			for (int i=0; i<(int) nfl.size(); ++i) 
			{
				if (EvaluateFaceVector(nfl[i].first, ntime, nvec, fv)) { r += fv; n++; }
			}
			r /= (float) n;
		}
	}
	else 
	{
//		assert(false);
		r = vec3f(0,0,0);
	}

	return r;
}


//-----------------------------------------------------------------------------
// Evaluate face vector data
bool FEModel::EvaluateFaceVector(int n, int ntime, int nvec, vec3f& r)
{
	FEState& state = *GetState(ntime);
	FEMeshBase* mesh = state.GetFEMesh();
	m_ntime = ntime;

	FEFace& f = mesh->Face(n);

	if (IS_FACE_FIELD(nvec))
	{
		// get the data ID
		int ndata = FIELD_CODE(nvec);
		if ((ndata < 0) || (ndata >= state.m_Data.size())) { r = vec3f(0.f, 0.f, 0.f); return false; }

		// get the component
		int ncomp = FIELD_COMP(nvec);

		FEMeshData& rd = state.m_Data[ndata];
		Data_Format fmt = rd.GetFormat(); 

		switch (rd.GetType())
		{
		case DATA_VEC3F:
			{
				switch (fmt)
				{
				case DATA_REGION:
					{
						FEFaceData_T<vec3f, DATA_REGION>& dv = dynamic_cast<FEFaceData_T<vec3f, DATA_REGION>&>(rd);
						if (dv.active(n))
						{
							dv.eval(n, &r);
							return true;
						}
					}
					break;
				case DATA_ITEM:
					{
						FEFaceData_T<vec3f,DATA_ITEM>& dv = dynamic_cast<FEFaceData_T<vec3f,DATA_ITEM>&>(rd);
						if (dv.active(n))
						{
							dv.eval(n, &r);
							return true;
						}
					}
					break;
				case DATA_NODE:
					{
						FEFaceData_T<vec3f,DATA_NODE>& dv = dynamic_cast<FEFaceData_T<vec3f,DATA_NODE>&>(rd);
						r = vec3f(0,0,0);
						if (dv.active(n))
						{
							vec3f v[FEFace::MAX_NODES];
							int fn = f.Nodes();
							dv.eval(n, v);
							for (int i=0; i<fn; ++i) r += v[i];
							r /= (float) fn;
						}
						return true;
					}
					break;
				case DATA_COMP:
					{
						vec3f rn[8];
						FEFaceData_T<vec3f,DATA_COMP>& dv = dynamic_cast<FEFaceData_T<vec3f,DATA_COMP>&>(rd);
						r = vec3f(0,0,0);
						if (dv.active(n))
						{
							dv.eval(n, rn);
							int ne = mesh->Face(n).Nodes();
							for (int i=0; i<ne; ++i) r += rn[i];
							r /= (float) ne;
							return true;
						}
					}
					break;
				}
			}
			break;
		case DATA_MAT3FS:
			{
				FEFaceData_T<mat3fs,DATA_ITEM>& dm = dynamic_cast<FEFaceData_T<mat3fs,DATA_ITEM>&>(rd);
				mat3fs m;
				dm.eval(n, &m);
				r = m.PrincDirection(ncomp+1);
				return true;
			}
			break;
		default:
			assert(false);
			return false;
		}
	}
	else if (IS_NODE_FIELD(nvec))
	{
		FEFace& f = mesh->Face(n);
		// take the average of the nodal values
		for (int i=0; i<f.Nodes(); ++i) r += EvaluateNodeVector(f.node[i], ntime, nvec);
		r /= f.Nodes();
		return true;
	}
	else if (IS_ELEM_FIELD(nvec))
	{
		return false;
	}
	else
	{
		assert(false);
		r = vec3f(0,0,0);
	}

	return false;
}

//-----------------------------------------------------------------------------
// Evaluate element vector data
vec3f FEModel::EvaluateElemVector(int n, int ntime, int nvec)
{
	FEState& state = *GetState(ntime);
	FEMeshBase* mesh = state.GetFEMesh();
	m_ntime = ntime;

	vec3f r;

	if (IS_ELEM_FIELD(nvec))
	{
		// get the data ID
		int ndata = FIELD_CODE(nvec);
		if ((ndata < 0) || (ndata >= state.m_Data.size())) return vec3f(0.f, 0.f, 0.f);

		// get the component
		int ncomp = FIELD_COMP(nvec);

		FEMeshData& rd = state.m_Data[ndata];

		int nfmt  = rd.GetFormat();

		switch (rd.GetType())
		{
		case DATA_VEC3F:
			{
				if (nfmt == DATA_ITEM)
				{
					FEElemData_T<vec3f,DATA_ITEM>& dv = dynamic_cast<FEElemData_T<vec3f,DATA_ITEM>&>(rd);
					if (dv.active(n)) dv.eval(n, &r);
				}
				else if (nfmt == DATA_COMP)
				{
					FEElemData_T<vec3f,DATA_COMP>& dv = dynamic_cast<FEElemData_T<vec3f,DATA_COMP>&>(rd);
					vec3f v[8];
					if (dv.active(n))
					{
						dv.eval(n, v);
						int ne = mesh->Element(n).Nodes();
						for (int i=0; i<ne; ++i) r += v[i];
						r /= (float) ne;
					}
				}
			}
			break;
		case DATA_MAT3FS:
			{
				if (nfmt == DATA_ITEM)
				{
					FEElemData_T<mat3fs,DATA_ITEM>& dm = dynamic_cast<FEElemData_T<mat3fs,DATA_ITEM>&>(rd);
					if (dm.active(n))
					{
						mat3fs m;
						dm.eval(n, &m);
						r = m.PrincDirection(ncomp+1);
					}
				}
				else if (nfmt == DATA_COMP)
				{
					FEElemData_T<mat3fs,DATA_COMP>& dm = dynamic_cast<FEElemData_T<mat3fs,DATA_COMP>&>(rd);
					mat3fs m[8];
					r = vec3f(0,0,0);
					if (dm.active(n))
					{
						dm.eval(n, m);
						int ne = mesh->Element(n).Nodes();
						for (int i=0; i<ne; ++i) r += m[i].PrincDirection(ncomp + 1);
						r /= (float) ne;
					}
				}
			}
			break;
		case DATA_MAT3F:
			{
				if (nfmt == DATA_ITEM)
				{
					FEElemData_T<mat3f, DATA_ITEM>& dm = dynamic_cast<FEElemData_T<mat3f, DATA_ITEM>&>(rd);
					if (dm.active(n))
					{
						mat3f m;
						dm.eval(n, &m);
						r = m.col(ncomp);
					}
				}
			}
			break;
		case DATA_ARRAY_VEC3F:
			{
				if (nfmt == DATA_ITEM)
				{
					FEElemArrayVec3Data& dm = dynamic_cast<FEElemArrayVec3Data&>(rd);
					if (dm.active(n))
					{
						r = dm.eval(n, ncomp);
					}
				}
			}
			break;
		default:
			assert(false);
		}
	}
	else if (IS_NODE_FIELD(nvec))
	{
		FEElement& el = mesh->Element(n);
		// take the average of the nodal values
		for (int i=0; i<el.Nodes(); ++i) r += EvaluateNodeVector(el.m_node[i], ntime, nvec);
		r /= el.Nodes();
	}
	else if (IS_FACE_FIELD(nvec))
	{
	}
	else
	{
		assert(false);
		r = vec3f(0,0,0);
	}

	return r;
}

//-----------------------------------------------------------------------------
// Evaluate tensor field at node n
mat3f FEModel::EvaluateNodeTensor(int n, int ntime, int nten, int ntype)
{
	FEState& state = *GetState(ntime);
	FEMeshBase* mesh = state.GetFEMesh();
	m_ntime = ntime;

	mat3f m;

	if (IS_NODE_FIELD(nten))
	{
		// get the data ID
		int ndata = FIELD_CODE(nten);
		if ((ndata < 0) || (ndata >= state.m_Data.size())) return mat3f();

		FEMeshData& rd = state.m_Data[ndata];
		if ((ntype != -1) && (rd.GetType() != ntype)) return m;

		switch (rd.GetType())
		{
		case DATA_MAT3FS:
			{
				FENodeData_T<mat3fs>& dm = dynamic_cast<FENodeData_T<mat3fs>&>(rd);
				mat3fs a;
				dm.eval(n, &a);
				m = a;
			}
			break;
		case DATA_MAT3F:
			{
				FENodeData_T<mat3f>& dm = dynamic_cast<FENodeData_T<mat3f>&>(rd);
				dm.eval(n, &m);
			}
			break;
		}
	}
	else 
	{
		// we take the average of the elements that contain this element
		vector<NodeElemRef>& nel = mesh->NodeElemList(n);
		if (!nel.empty())
		{
			for (int i=0; i<(int) nel.size(); ++i) m += EvaluateElemTensor(nel[i].first, ntime, nten, ntype);
			m /= (float) nel.size();
		}
	}

	return m;
}

//-----------------------------------------------------------------------------
// Evaluate tensor field at face n
mat3f FEModel::EvaluateFaceTensor(int n, int ntime, int nten, int ntype)
{
	FEState& state = *GetState(ntime);
	FEMeshBase* mesh = state.GetFEMesh();
	m_ntime = ntime;

	mat3f m;

	if (IS_FACE_FIELD(nten))
	{
		// get the data ID
		int ndata = FIELD_CODE(nten);
		if ((ndata < 0) || (ndata >= state.m_Data.size())) return mat3f();
		FEMeshData& rd = state.m_Data[ndata];
		if ((ntype != -1) && (rd.GetType() != ntype)) return m;

		switch (rd.GetType())
		{
		case DATA_MAT3FS:
			{
				mat3fs a;
				FEFaceData_T<mat3fs,DATA_ITEM>& dm = dynamic_cast<FEFaceData_T<mat3fs,DATA_ITEM>&>(rd);
				dm.eval(n, &a);
				m = a;
			}
			break;
		case DATA_MAT3F:
			{
				FEFaceData_T<mat3f, DATA_ITEM>& dm = dynamic_cast<FEFaceData_T<mat3f, DATA_ITEM>&>(rd);
				dm.eval(n, &m);
			}
			break;
		}
	}
	else if (IS_NODE_FIELD(nten))
	{
		FEFace& f = mesh->Face(n);
		// take the average of the nodal values
		for (int i = 0; i<f.Nodes(); ++i) m += EvaluateNodeTensor(f.node[i], ntime, nten, ntype);
		m /= (float) f.Nodes();
	}
	else if (IS_ELEM_FIELD(nten))
	{
		
	}
	else
	{
		assert(false);
	}

	return m;
}

//-----------------------------------------------------------------------------
// Evaluate tensor field at element n
mat3f FEModel::EvaluateElemTensor(int n, int ntime, int nten, int ntype)
{
	FEState& state = *GetState(ntime);
	FEMeshBase* mesh = state.GetFEMesh();
	m_ntime = ntime;

	mat3f m;

	if (IS_ELEM_FIELD(nten))
	{
		// get the data ID
		int ndata = FIELD_CODE(nten);
		if ((ndata < 0) || (ndata >= state.m_Data.size())) return mat3f();
		FEMeshData& rd = state.m_Data[ndata];
		int nfmt  = rd.GetFormat();

		if ((ntype != -1) && (rd.GetType() != ntype)) return m;

		switch (nfmt)
		{
		case DATA_ITEM:
			{
				switch (rd.GetType())
				{
				case DATA_MAT3FS:
					{
						FEElemData_T<mat3fs,DATA_ITEM>& dm = dynamic_cast<FEElemData_T<mat3fs,DATA_ITEM>&>(rd);
						if (dm.active(n))
						{
							mat3fs a;
							dm.eval(n, &a);
							m = a;
						}
					}
					break;
				case DATA_MAT3F:
					{
						FEElemData_T<mat3f, DATA_ITEM>& dm = dynamic_cast<FEElemData_T<mat3f, DATA_ITEM>&>(rd);
						if (dm.active(n)) dm.eval(n, &m);
					}
					break;
				}
			}
			break;
		case DATA_COMP:
			{
				switch (rd.GetType())
				{
				case DATA_MAT3FS:
					{
						FEElemData_T<mat3fs,DATA_COMP>& dm = dynamic_cast<FEElemData_T<mat3fs,DATA_COMP>&>(rd);
						mat3fs mi[FEGenericElement::MAX_NODES];
						if (dm.active(n))
						{
							dm.eval(n, mi);
							int ne = mesh->Element(n).Nodes();
							mat3fs a = mi[0];
							for (int i=1; i<ne; ++i) a += mi[i];
							a /= (float) ne;

							m = a;
						}
					}
					break;
				case DATA_MAT3F:
					{
						FEElemData_T<mat3f, DATA_COMP>& dm = dynamic_cast<FEElemData_T<mat3f, DATA_COMP>&>(rd);
						mat3f mi[FEGenericElement::MAX_NODES];
						if (dm.active(n))
						{
							dm.eval(n, mi);
							int ne = mesh->Element(n).Nodes();
							m = mi[0];
							for (int i = 1; i<ne; ++i) m += mi[i];
							m /= (float)ne;
						}
					}
					break;
				}
			};
			break;
		}
	}
	else if (IS_NODE_FIELD(nten))
	{
		FEElement& el = mesh->Element(n);
		// take the average of the nodal values
		for (int i = 0; i<el.Nodes(); ++i) m += EvaluateNodeTensor(el.m_node[i], ntime, nten, ntype);
		m /= (float) el.Nodes();
	}
	else if (IS_FACE_FIELD(nten))
	{
		
	}
	else
	{
	}

	return m;
}
