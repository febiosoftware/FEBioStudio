#include "GPrimitive.h"
#include <MeshTools/FEGregoryPatch.h>
#include <MeshTools/FETetGenMesher.h>

//=============================================================================
// GPrimitive
//=============================================================================

//-----------------------------------------------------------------------------

bool GPrimitive::Update(bool b)
{
	// I don't think we ever come here anymore
	// If so, we can delete this function
	assert(false);
	return false;
}

//-----------------------------------------------------------------------------
void GPrimitive::Save(OArchive &ar)
{
	// save the name
	ar.WriteChunk(CID_OBJ_NAME, GetName());

	// save the transform stuff
	ar.BeginChunk(CID_OBJ_HEADER);
	{
		int nid = GetID();
		ar.WriteChunk(CID_OBJ_ID, nid);
		ar.WriteChunk(CID_OBJ_POS, Transform().GetPosition());
		ar.WriteChunk(CID_OBJ_ROT, Transform().GetRotation());
		ar.WriteChunk(CID_OBJ_SCALE, Transform().GetScale());
		ar.WriteChunk(CID_OBJ_COLOR, GetColor());

		int nparts = Parts();
		int nfaces = Faces();
		int nedges = Edges();
		int nnodes = Nodes();

		ar.WriteChunk(CID_OBJ_PARTS, nparts);
		ar.WriteChunk(CID_OBJ_FACES, nfaces);
		ar.WriteChunk(CID_OBJ_EDGES, nedges);
		ar.WriteChunk(CID_OBJ_NODES, nnodes);
	}
	ar.EndChunk();

	// save the parameters
	if (Parameters() > 0)
	{
		ar.BeginChunk(CID_OBJ_PARAMS);
		{
			ParamContainer::Save(ar);
		}
		ar.EndChunk();
	}

	// save the parts
	ar.BeginChunk(CID_OBJ_PART_SECTION);
	{
		for (int i=0; i<Parts(); ++i)
		{
			ar.BeginChunk(CID_OBJ_PART);
			{
				GPart& p = *Part(i);
				int nid = p.GetID();
				int mid = p.GetMaterialID();
				ar.WriteChunk(CID_OBJ_PART_ID, nid);
				ar.WriteChunk(CID_OBJ_PART_MAT, mid);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the surfaces
	ar.BeginChunk(CID_OBJ_FACE_SECTION);
	{
		for (int i=0; i<Faces(); ++i)
		{
			ar.BeginChunk(CID_OBJ_FACE);
			{
				GFace& f = *Face(i);
				int nid = f.GetID();
				ar.WriteChunk(CID_OBJ_FACE_ID, nid);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the edges
	ar.BeginChunk(CID_OBJ_EDGE_SECTION);
	{
		for (int i=0; i<Edges(); ++i)
		{
			ar.BeginChunk(CID_OBJ_EDGE);
			{
				GEdge& e = *Edge(i);
				int nid = e.GetID();
				ar.WriteChunk(CID_OBJ_EDGE_ID, nid);
			}
			ar.EndChunk();
		}
	}
	ar.EndChunk();

	// save the nodes
	// note that it is possible that an object doesn't have any nodes
	// for instance, a shell disc
	if (Nodes()>0)
	{
		ar.BeginChunk(CID_OBJ_NODE_SECTION);
		{
			for (int i=0; i<Nodes(); ++i)
			{	
				ar.BeginChunk(CID_OBJ_NODE);
				{
					GNode& v = *Node(i);
					int nid = v.GetID();
					ar.WriteChunk(CID_OBJ_NODE_ID, nid);
					ar.WriteChunk(CID_OBJ_NODE_POS, v.LocalPosition());
				}
				ar.EndChunk();
			}
		}
		ar.EndChunk();
	}

	// save the mesher object
	if (m_pMesher)
	{
		ar.BeginChunk(CID_OBJ_FEMESHER);
		{
			int ntype = 0;
			if (dynamic_cast<FETetGenMesher*>(m_pMesher)) ntype = 1;
			ar.BeginChunk(ntype);
			{
				m_pMesher->Save(ar);
			}
			ar.EndChunk();
		}
		ar.EndChunk();
	}

	// save the mesh
	if (m_pmesh)
	{
		ar.BeginChunk(CID_MESH);
		{
			m_pmesh->Save(ar);
		}
		ar.EndChunk();
	}
}

//-----------------------------------------------------------------------------
void GPrimitive::Load(IArchive& ar)
{
	TRACE("GPrimitive::Load");

	int nparts = -1, nfaces = -1, nedges = -1, nnodes = -1;

	while (IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
		// object name
		case CID_OBJ_NAME: 
			{
				char sz[256];
				ar.read(sz);
				SetName(sz);
			}
			break;
		// header
		case CID_OBJ_HEADER:
			{
				vec3d pos, scl;
				quatd rot;
				GLCOLOR col;
				while (IO_OK == ar.OpenChunk())
				{
					int nid = ar.GetChunkID();
					int oid;
					switch (nid)
					{
					case CID_OBJ_ID: ar.read(oid); SetID(oid); break;
					case CID_OBJ_POS: ar.read(pos); break;
					case CID_OBJ_ROT: ar.read(rot); break;
					case CID_OBJ_SCALE: ar.read(scl); break;
					case CID_OBJ_COLOR: ar.read(col); break;
					case CID_OBJ_PARTS: ar.read(nparts); break;
					case CID_OBJ_FACES: ar.read(nfaces); break;
					case CID_OBJ_EDGES: ar.read(nedges); break;
					case CID_OBJ_NODES: ar.read(nnodes); break;
					}
					ar.CloseChunk();
				}

				SetColor(col);

				// update transform
				GTransform& transform = Transform();
				transform.SetPosition(pos);
				transform.SetRotation(rot);
				transform.SetScale(scl);
			}
			break;
		// object parameters
		case CID_OBJ_PARAMS:
			ParamContainer::Load(ar);
			break;
		// object parts
		case CID_OBJ_PART_SECTION:
			{
				assert(nparts == m_Part.size());
				int n = 0;
				while (IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_PART) throw ReadError("error parsing CID_OBJ_PART_SECTION (GPrimitive::Load)");

					GPart& p = *m_Part[n];
					while (IO_OK == ar.OpenChunk())
					{
						int nid, mid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_PART_ID : ar.read(nid); p.SetID(nid); break;
						case CID_OBJ_PART_MAT: ar.read(mid); p.SetMaterialID(mid); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					p.SetLocalID(n++);
				}
			}
			break;
		// object surfaces
		case CID_OBJ_FACE_SECTION:
			{
				assert(nfaces == m_Face.size());
				int n = 0;
				while (IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_FACE) throw ReadError("error parsing CID_OBJ_FACE_SECTION (GPrimitive::Load)");

					GFace& f = *m_Face[n];
					while (IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_FACE_ID: ar.read(nid); f.SetID(nid); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					f.SetLocalID(n++);
				}
			}
			break;
		// object edges
		case CID_OBJ_EDGE_SECTION:
			{
				assert(nedges == m_Edge.size());
				int n = 0;
				while (IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_EDGE) throw ReadError("error parsing CID_OBJ_EDGE_SECTION (GPrimitive::Load)");

					GEdge& e = *m_Edge[n];
					while (IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_EDGE_ID: ar.read(nid); e.SetID(nid); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					e.SetLocalID(n++);
				}
			}
			break;
		// object nodes
		case CID_OBJ_NODE_SECTION:
			{
				assert(nnodes == m_Node.size());
				int m = 0;
				while (IO_OK == ar.OpenChunk())
				{
					if (ar.GetChunkID() != CID_OBJ_NODE) throw ReadError("error parsing CID_OBJ_NODE_SECTION (GPrimitive::Load)");

					GNode& n = *m_Node[m];
					while (IO_OK == ar.OpenChunk())
					{
						int nid;
						switch (ar.GetChunkID())
						{
						case CID_OBJ_NODE_ID: ar.read(nid); n.SetID(nid); break;
						case CID_OBJ_NODE_POS: ar.read(n.LocalPosition()); break;
						}
						ar.CloseChunk();
					}
					ar.CloseChunk();

					n.SetLocalID(m++);

				}
			}
			break;
		// mesher object (obsolete way)
		case CID_OBJ_DEFAULT_MESHER:
			if (ar.Version() > 0x00010005) throw ReadError("error parsing CID_OBJ_DEFAULT_MESHER (GPrimitive::Load)");
			assert(m_pMesher);
			m_pMesher->Load(ar);
			break;
		// mesher object (new way)
		case CID_OBJ_FEMESHER:
			{
				if (ar.OpenChunk() != IO_OK) throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
				else
				{
					int ntype = ar.GetChunkID();
	//				assert(m_pMesher == 0);
					switch (ntype)
					{
					case 0: break;	// use default mesher
					case 1: SetFEMesher(new FETetGenMesher(this)); break;
					default:
						throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
					}
					m_pMesher->Load(ar);
				}
				ar.CloseChunk();
				if (ar.OpenChunk() != IO_END) throw ReadError("error parsing CID_OBJ_FEMESHER (GPrimitive::Load)");
			}
			break;
		// the mesh object
		case CID_MESH:
			if (m_pmesh) delete m_pmesh;
			SetFEMesh(new FEMesh);
			m_pmesh->Load(ar);
			break;
		}
		ar.CloseChunk();
	}

	Update(false);
}

//-----------------------------------------------------------------------------
extern GObject* BuildObject(int ntype);

//-----------------------------------------------------------------------------
GObject* GPrimitive::Clone()
{
	// create a new copy of this primitive
	GObject* po = BuildObject(GetType());
	assert(po);
	if (po == 0) return 0;

	// copy transform
	po->CopyTransform(this);

	// copy color
	po->SetColor(GetColor());

	// copy parameters
	po->GetParamBlock() = GetParamBlock();

	// update the object
	po->Update();

	return po;
}

//=============================================================================
// GGregoryPatch
//=============================================================================

void GGregoryPatch::UpdateMesh()
{
	FEGregoryPatch& m = dynamic_cast<FEGregoryPatch&>(*m_pmesh);

	// reposition the nodes
	for (int i=0; i<Nodes(); ++i)
	{
		GNode& n = *Node(i);
		FEGregoryPatch::GNode& gn = m.GetNode(i);
		gn.m_r = n.LocalPosition();
	}

	// rebuild the FE mesh
	m.BuildPatchData();
}
