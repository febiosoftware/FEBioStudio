/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2021 University of Utah, The Trustees of Columbia University in
the City of New York, and others.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/

#include "FEConnector.h"
#include "FSModel.h"
#include <GeomLib/GGroup.h>
using namespace std;

//=============================================================================
// FSRigidConnector
//-----------------------------------------------------------------------------

FSRigidConnector::FSRigidConnector(int ntype, FSModel* ps, int nstep) : FSStepComponent(ps)
{
    m_ntype = ntype;
	SetStep(nstep);
    m_bActive = true;
	SetSuperClassID(FENLCONSTRAINT_ID);
}

//-----------------------------------------------------------------------------
FSRigidConnector::~FSRigidConnector()
{
    
}

//-----------------------------------------------------------------------------
void FSRigidConnector::SetPosition(const vec3d& r)
{

}

//-----------------------------------------------------------------------------
int FSRigidConnector::Type()
{ 
	return m_ntype; 
}

//-----------------------------------------------------------------------------
void FSRigidConnector::SaveList(FSItemListBuilder* pitem, OArchive& ar)
{
    if (pitem)
    {
        ar.BeginChunk((int) pitem->Type());
        pitem->Save(ar);
        ar.EndChunk();
    }
}

//-----------------------------------------------------------------------------
FSItemListBuilder* FSRigidConnector::LoadList(IArchive& ar)
{
	FSItemListBuilder* pitem = 0;

    FSModel* fem = GetFSModel();
	GModel* gm = &fem->GetModel();
    
    if (ar.OpenChunk() != IArchive::IO_OK) throw ReadError("error in FSRigidConnector::LoadList");
    unsigned int ntype = ar.GetChunkID();
    switch (ntype)
    {
        case GO_NODE: pitem = new GNodeList(gm); break;
        case GO_EDGE: pitem = new GEdgeList(gm); break;
        case GO_FACE: pitem = new GFaceList(gm); break;
        case GO_PART: pitem = new GPartList(gm); break;
        case FE_NODESET: pitem = new FSNodeSet(nullptr); break;
        case FE_EDGESET: pitem = new FSEdgeSet(nullptr); break;
        case FE_SURFACE: pitem = new FSSurface(nullptr); break;
        case FE_ELEMSET: pitem = new FSElemSet(nullptr); break;
        default:
            assert(false);
    }
    if (pitem == 0) throw ReadError("unknown item list type (FSInterface::LoadList)");
    
    pitem->Load(ar);
    ar.CloseChunk();
    
    int nret = ar.OpenChunk();
    if (nret != IArchive::IO_END) throw ReadError("error in FSInterface::LoadList");
    
    // set the parent mesh for FSGroup's
    FSGroup* pg = dynamic_cast<FSGroup*>(pitem);
    if (pg)
    {
        if (fem->FindGroupParent(pg) == false) throw ReadError("Invalid object ID in FSInterface::Load");
    }
    
    return pitem;
}

//=============================================================================
// FBSRigidConnector
//-----------------------------------------------------------------------------
FBSRigidConnector::FBSRigidConnector(int ntype, FSModel* ps, int nstep) : FSRigidConnector(ntype, ps, nstep)
{
    m_rbA = m_rbB = -1;
}

//-----------------------------------------------------------------------------
void FBSRigidConnector::Save(OArchive& ar)
{
    ar.WriteChunk(CID_CONNECTOR_NAME, GetName());
    ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
    ar.WriteChunk(CID_CONNECTOR_ACTIVE, m_bActive);
    ar.WriteChunk(CID_CONNECTOR_STEP, GetStep());
    ar.BeginChunk(CID_CONNECTOR_PARAMS);
    {
        ParamContainer::Save(ar);
    }
    ar.EndChunk();

    ar.WriteChunk(CID_RC_RIGIDBODY_A, m_rbA);
    ar.WriteChunk(CID_RC_RIGIDBODY_B, m_rbB);
}

//-----------------------------------------------------------------------------
void FBSRigidConnector::Load(IArchive& ar)
{
    TRACE("FSRigidConnector::Load");
    while (IArchive::IO_OK == ar.OpenChunk())
    {
        switch (ar.GetChunkID())
        {
        case CID_CONNECTOR_NAME: { string name; ar.read(name); SetName(name); } break;
        case CID_FEOBJ_INFO: { string info; ar.read(info); SetInfo(info); } break;
        case CID_CONNECTOR_ACTIVE: ar.read(m_bActive); break;
        case CID_CONNECTOR_STEP: { int nstep; ar.read(nstep); SetStep(nstep); } break;
        case CID_CONNECTOR_PARAMS: ParamContainer::Load(ar); break;
        case CID_RC_RIGIDBODY_A: ar.read(m_rbA); break;
        case CID_RC_RIGIDBODY_B: ar.read(m_rbB); break;
        default:
            throw ReadError("unknown CID in FSRigidConnector::Load");
        }
        ar.CloseChunk();
    }
}

//=============================================================================
// FSRigidSphericalJoint
//-----------------------------------------------------------------------------

FSRigidSphericalJoint::FSRigidSphericalJoint(FSModel* ps, int nstep) : FBSRigidConnector(FE_RC_SPHERICAL_JOINT, ps, nstep)
{
    SetTypeString("rigid spherical joint");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddIntParam   (1           , "auto_penalty"         , "auto penalty"         );
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"         );
    AddIntParam   (0           , "minaug"               , "minimum augmentations");
    AddIntParam   (10          , "maxaug"               , "maximum augmentations");
    AddBoolParam  (0           , "prescribed_rotation"  , "prescribed rotation flag");
    AddDoubleParam(0           , "rotation_x"           , "x-rotation");
    AddDoubleParam(0           , "rotation_y"           , "y-rotation");
    AddDoubleParam(0           , "rotation_z"           , "z-rotation");
    AddDoubleParam(0           , "moment_x"             , "x-moment");
    AddDoubleParam(0           , "moment_y"             , "y-moment");
    AddDoubleParam(0           , "moment_z"             , "z-moment");
}

//-----------------------------------------------------------------------------
void FSRigidSphericalJoint::SetPosition(const vec3d& r)
{
	SetVecValue(J_ORIG, r);
}

//=============================================================================
// FSRigidRevoluteJoint
//-----------------------------------------------------------------------------

FSRigidRevoluteJoint::FSRigidRevoluteJoint(FSModel* ps, int nstep) : FBSRigidConnector(FE_RC_REVOLUTE_JOINT, ps, nstep)
{
    SetTypeString("rigid revolute joint");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddIntParam   (1           , "auto_penalty"         , "auto penalty"         );
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"        );
    AddVecParam   (vec3d(0,0,0), "rotation_axis"        , "rotation axis"       );
    AddVecParam   (vec3d(0,0,0), "transverse_axis"      , "transverse axis"     );
    AddIntParam   (0           , "minaug"              , "minimum augmentations");
    AddIntParam   (10          , "maxaug"              , "maximum augmentations");
    AddBoolParam  (0           , "prescribed_rotation"  , "prescribed rotation flag");
    AddDoubleParam(0           , "rotation"             , "rotation angle");
    AddDoubleParam(0           , "moment"               , "prescribed moment");
}

//-----------------------------------------------------------------------------
void FSRigidRevoluteJoint::SetPosition(const vec3d& r)
{
	SetVecValue(J_ORIG, r);
}

//=============================================================================
// FSRigidPrismaticJoint
//-----------------------------------------------------------------------------

FSRigidPrismaticJoint::FSRigidPrismaticJoint(FSModel* ps, int nstep) : FBSRigidConnector(FE_RC_PRISMATIC_JOINT, ps, nstep)
{
    SetTypeString("rigid prismatic joint");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddIntParam   (0           , "auto_penalty"         , "auto penalty"         );
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"       );
    AddVecParam   (vec3d(0,0,0), "translation_axis"     , "translation axis"   );
    AddVecParam   (vec3d(0,0,0), "transverse_axis"      , "transverse axis"    );
    AddIntParam   (0           , "minaug"              , "minimum augmentations");
    AddIntParam   (10          , "maxaug"              , "maximum augmentations");
    AddBoolParam  (0           , "prescribed_translation", "prescribed translation flag");
    AddDoubleParam(0           , "translation"          , "translation");
    AddDoubleParam(0           , "force"             , "prescribed force");
}

//-----------------------------------------------------------------------------
void FSRigidPrismaticJoint::SetPosition(const vec3d& r)
{
	SetVecValue(J_ORIG, r);
}

//=============================================================================
// FSRigidCylindricalJoint
//-----------------------------------------------------------------------------

FSRigidCylindricalJoint::FSRigidCylindricalJoint(FSModel* ps, int nstep) : FBSRigidConnector(FE_RC_CYLINDRICAL_JOINT, ps, nstep)
{
    SetTypeString("rigid cylindrical joint");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddIntParam   (1           , "auto_penalty"         , "auto penalty"         );
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"        );
    AddVecParam   (vec3d(0,0,0), "joint_axis"           , "joint axis"          );
    AddVecParam   (vec3d(0,0,0), "transverse_axis"      , "transverse axis"     );
    AddIntParam   (0           , "minaug"              , "minimum augmentations");
    AddIntParam   (10          , "maxaug"              , "maximum augmentations");
    AddBoolParam  (0           , "prescribed_translation", "prescribed translation flag");
    AddDoubleParam(0           , "translation"          , "translation");
    AddDoubleParam(0           , "force"             , "prescribed force");
    AddBoolParam  (0           , "prescribed_rotation"  , "prescribed rotation flag");
    AddDoubleParam(0           , "rotation"             , "rotation angle");
    AddDoubleParam(0           , "moment"               , "prescribed moment");
}

//-----------------------------------------------------------------------------
void FSRigidCylindricalJoint::SetPosition(const vec3d& r)
{
	SetVecValue(J_ORIG, r);
}

//=============================================================================
// FSRigidPlanarJoint
//-----------------------------------------------------------------------------

FSRigidPlanarJoint::FSRigidPlanarJoint(FSModel* ps, int nstep) : FBSRigidConnector(FE_RC_PLANAR_JOINT, ps, nstep)
{
    SetTypeString("rigid planar joint");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddIntParam   (1           , "auto_penalty"         , "auto penalty"         );
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"        );
    AddVecParam   (vec3d(0,0,0), "rotation_axis"        , "rotation axis"       );
    AddVecParam   (vec3d(0,0,0), "translation_axis_1"   , "translation axis 1"  );
    AddIntParam   (0           , "minaug"              , "minimum augmentations");
    AddIntParam   (10          , "maxaug"              , "maximum augmentations");
    AddBoolParam  (0           , "prescribed_translation_1", "prescribed translation flag along axis 1");
    AddDoubleParam(0           , "translation_1"        , "translation 1");
    AddBoolParam  (0           , "prescribed_translation_2", "prescribed translation flag along axis 2");
    AddDoubleParam(0           , "translation_2"        , "translation 2");
    AddBoolParam  (0           , "prescribed_rotation"  , "prescribed rotation flag");
    AddDoubleParam(0           , "rotation"             , "rotation angle");
}

//-----------------------------------------------------------------------------
void FSRigidPlanarJoint::SetPosition(const vec3d& r)
{
	SetVecValue(J_ORIG, r);
}

//=============================================================================
// FSRigidLock
//-----------------------------------------------------------------------------

FSRigidLock::FSRigidLock(FSModel* ps, int nstep) : FBSRigidConnector(FE_RC_RIGID_LOCK, ps, nstep)
{
    SetTypeString("rigid lock");
    
    AddDoubleParam(0.1         , "tolerance"            , "tolerance");
    AddDoubleParam(0           , "gaptol"               , "gap tolerance");
    AddDoubleParam(0           , "angtol"               , "angular tolerance");
    AddDoubleParam(1           , "force_penalty"        , "force penalty factor");
    AddDoubleParam(0           , "moment_penalty"       , "moment penalty factor");
    AddIntParam   (1           , "auto_penalty"         , "auto penalty"         );
    AddVecParam   (vec3d(0,0,0), "joint_origin"         , "joint origin"         );
    AddVecParam   (vec3d(0,0,0), "first_axis"           , "first axis"           );
    AddVecParam   (vec3d(0,0,0), "second_axis"          , "second axis"          );
    AddIntParam   (0           , "minaug"               , "minimum augmentations");
    AddIntParam   (10          , "maxaug"               , "maximum augmentations");
}

//-----------------------------------------------------------------------------
void FSRigidLock::SetPosition(const vec3d& r)
{
    SetVecValue(J_ORIG, r);
}

//=============================================================================
// FSRigidSpring
//-----------------------------------------------------------------------------

FSRigidSpring::FSRigidSpring(FSModel* ps, int nstep) : FBSRigidConnector(FE_RC_SPRING, ps, nstep)
{
    SetTypeString("rigid spring");
    
    AddDoubleParam(0           , "k", "stiffness");
    AddVecParam   (vec3d(0,0,0), "insertion_a" , "insertion point a");
    AddVecParam   (vec3d(0,0,0), "insertion_b" , "insertion point b");
}

//=============================================================================
// FSRigidDamper
//-----------------------------------------------------------------------------

FSRigidDamper::FSRigidDamper(FSModel* ps, int nstep) : FBSRigidConnector(FE_RC_DAMPER, ps, nstep)
{
    SetTypeString("rigid damper");
    
    AddDoubleParam(0           , "c", "damping");
    AddVecParam   (vec3d(0,0,0), "insertion_a" , "insertion point a");
    AddVecParam   (vec3d(0,0,0), "insertion_b" , "insertion point b");
}

//=============================================================================
// FSRigidAngularDamper
//-----------------------------------------------------------------------------

FSRigidAngularDamper::FSRigidAngularDamper(FSModel* ps, int nstep) : FBSRigidConnector(FE_RC_ANGULAR_DAMPER, ps, nstep)
{
    SetTypeString("rigid angular damper");
    
    AddDoubleParam(0.0 , "c", "damping");
}

//=============================================================================
// FSRigidContractileForce
//-----------------------------------------------------------------------------

FSRigidContractileForce::FSRigidContractileForce(FSModel* ps, int nstep) : FBSRigidConnector(FE_RC_CONTRACTILE_FORCE, ps, nstep)
{
    SetTypeString("rigid contractile force");
    
    AddDoubleParam(0           , "f0", "force");
    AddVecParam   (vec3d(0,0,0), "insertion_a" , "insertion point a");
    AddVecParam   (vec3d(0,0,0), "insertion_b" , "insertion point b");
}

//=============================================================================
// FSGenericRigidJoint
//-----------------------------------------------------------------------------

FSGenericRigidJoint::FSGenericRigidJoint(FSModel* ps, int nstep) : FBSRigidConnector(FE_RC_GENERIC_JOINT, ps, nstep)
{
	SetTypeString("generic rigid joint");

	AddChoiceParam(0, "laugon", "Enforcement method")->SetEnumNames("Penalty method\0Augmented Lagrangian\0Lagrange multiplier\0");
	AddDoubleParam(0, "penalty", "Penalty");
	AddDoubleParam(0, "tolerance", "Tolerance");
	AddVecParam(vec3d(0, 0, 0), "joint", "joint position");
	AddDoubleParam(0, "prescribe_x" , "x" )->SetCheckable(true);
	AddDoubleParam(0, "prescribe_y" , "y" )->SetCheckable(true);
	AddDoubleParam(0, "prescribe_z" , "z" )->SetCheckable(true);
	AddDoubleParam(0, "prescribe_Rx", "Rx")->SetCheckable(true);
	AddDoubleParam(0, "prescribe_Ry", "Ry")->SetCheckable(true);
	AddDoubleParam(0, "prescribe_Rz", "Rz")->SetCheckable(true);
}

//=============================================================================
FEBioRigidConnector::FEBioRigidConnector(FSModel* ps, int nstep) : FSRigidConnector(FE_FEBIO_RIGID_CONNECTOR, ps, nstep)
{

}

void FEBioRigidConnector::SetRigidBody1(int rb) 
{ 
    SetParamInt("body_a", rb);
}

void FEBioRigidConnector::SetRigidBody2(int rb) 
{ 
    SetParamInt("body_b", rb);
}

int FEBioRigidConnector::GetRigidBody1() const
{ 
    return GetParam("body_a")->GetIntValue();
}

int FEBioRigidConnector::GetRigidBody2() const
{ 
    return GetParam("body_b")->GetIntValue();
}

void FEBioRigidConnector::Save(OArchive& ar)
{
    ar.BeginChunk(CID_FEBIO_META_DATA);
    {
        SaveClassMetaData(this, ar);
    }
    ar.EndChunk();

    ar.BeginChunk(CID_FEBIO_BASE_DATA);
    {
        ar.WriteChunk(CID_CONNECTOR_NAME, GetName());
        ar.WriteChunk(CID_FEOBJ_INFO, GetInfo());
        ar.WriteChunk(CID_CONNECTOR_ACTIVE, m_bActive);
        ar.WriteChunk(CID_CONNECTOR_STEP, GetStep());
        ar.BeginChunk(CID_CONNECTOR_PARAMS);
        {
            ParamContainer::Save(ar);
        }
        ar.EndChunk();
    }
    ar.EndChunk();
}

void FEBioRigidConnector::Load(IArchive& ar)
{
    TRACE("FEBioRigidConnector::Load");
    while (IArchive::IO_OK == ar.OpenChunk())
    {
        int nid = ar.GetChunkID();
        switch (nid)
        {
        case CID_FEBIO_META_DATA: LoadClassMetaData(this, ar); break;
        case CID_FEBIO_BASE_DATA: 
        {
            while (IArchive::IO_OK == ar.OpenChunk())
            {
                switch (ar.GetChunkID())
                {
                case CID_CONNECTOR_NAME  : { string name; ar.read(name); SetName(name); } break;
                case CID_FEOBJ_INFO      : { string info; ar.read(info); SetInfo(info); } break;
                case CID_CONNECTOR_ACTIVE: ar.read(m_bActive); break;
                case CID_CONNECTOR_STEP  : { int nstep; ar.read(nstep); SetStep(nstep); } break;
                case CID_CONNECTOR_PARAMS: ParamContainer::Load(ar); break;
                default:
                    throw ReadError("unknown CID in FEBioRigidConnector::Load");
                }
                ar.CloseChunk();
            }
        }
        break;
        default:
            assert(false);
        }
        ar.CloseChunk();
    }
}
