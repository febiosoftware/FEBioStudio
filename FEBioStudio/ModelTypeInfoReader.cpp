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

#include "stdafx.h"
#include "ModelTypeInfoReader.h"
#include <FSCore/Serializable.h>
#include <MeshTools/FEProject.h>
#include <MeshTools/FEModel.h>
#include <FEMLib/FEMaterialFactory.h>
#include <FEMLib/FEMKernel.h>
#include <FEMLib/FEBoundaryCondition.h>
#include "version.h"
#include <sstream>

#include <iostream>

using std::stringstream;


bool ModelTypeInfoReader::Load(const char* szfile)
{
    if (!PRVArchive::Load(szfile))
	{
		return errf("Failed opening file");
	}
	else
	{
		try
		{
			IArchive& ar = GetArchive();

            CCallStack::ClearStack();
            TRACE("CDocument::Load");
            unsigned int version;

            IArchive::IOResult nret = IArchive::IO_OK;
            while (ar.OpenChunk() == IArchive::IO_OK)
            {
                int nid = ar.GetChunkID();

                if (nid == CID_VERSION)
                {
                    nret = ar.read(version);
                    if (nret != IArchive::IO_OK) throw ReadError("Error occurred when parsing CID_VERSION (CDocument::Load)");
                    if (version < MIN_FSM_VERSION)
                    {
                        throw InvalidVersion();
                    }
                    else if (version < SAVE_VERSION)
                    {
        //				int nret = flx_choice("This file was created with an older version of PreView.\nThe file data may not be read in correctly.\n\nWould you like to continue?", "Yes", "No", 0);
        //				if (nret != 0) throw InvalidVersion();
                    }
                    else if (version > SAVE_VERSION)
                    {
                        throw InvalidVersion();
                    }

                    // set the version number of the file
                    ar.SetVersion(version);
                }
                else if (nid == CID_PROJECT)
                {
                    ReadProject(ar);
                }
                ar.CloseChunk();

            }

			std::string log = ar.GetLog();
			if (log.empty() == false)
			{
				errf("%s", log.c_str());
			}

			Close();

            for(auto& item : typeInfo)
            {
                std::cout << item.first << std::endl;
                for(auto& str : item.second)
                {
                    std::cout << "\t" << str << std::endl;
                }
            }

			return true;
		}
		catch (InvalidVersion)
		{
			Close();
			return errf("This file has an invalid version number.");
		}
		catch (ReadError e)
		{
			char* sz = 0;
			int L = CCallStack::GetCallStackString(0);
			sz = new char[L + 1];
			CCallStack::GetCallStackString(sz);

			stringstream ss;
			if (e.m_szmsg) { ss << e.m_szmsg << "\n"; }
			else { ss << "(unknown)\n"; };
			ss << "\nCALL STACK:\n" << sz;
			delete[] sz;

			string errMsg = ss.str();

			Close();
			return errf(errMsg.c_str());
		}
		catch (std::exception e)
		{
			Close();
			const char* szerr = (e.what() == nullptr ? "(unknown)" : e.what());
			return errf("An exception occurred: %s", szerr);
		}
		catch (...)
		{
			Close();
			return errf("Failed opening file %s", szfile);
		}
	}

	Close();
	return false;
}

void ModelTypeInfoReader::ReadProject(IArchive& ar)
{
    TRACE("FSProject::Load");

    typeInfo["Module"] = unordered_set<string>();

    while (IArchive::IO_OK == ar.OpenChunk())
    {
        int nid = ar.GetChunkID();
        switch (nid)
        {
        case CID_PRJ_MODULES: 
        {
            int oldModuleId = 0;  
            ar.read(oldModuleId);

            if (oldModuleId & MODULE_FLUID_FSI  ) typeInfo["Module"].insert("fluid-FSI");
            if (oldModuleId & MODULE_FLUID      ) typeInfo["Module"].insert("fluid");
            if (oldModuleId & MODULE_MULTIPHASIC) typeInfo["Module"].insert("multiphasic");
            if (oldModuleId & MODULE_BIPHASIC   ) typeInfo["Module"].insert("biphasic");
            if (oldModuleId & MODULE_HEAT       ) typeInfo["Module"].insert("heat");
            if (oldModuleId & MODULE_MECH       ) typeInfo["Module"].insert("solid");

            break;
        } 
        case CID_PRJ_MODULE_NAME:
        {
            string modName;
            ar.read(modName);

            typeInfo["Module"].insert(modName);
        }
        break;
        case CID_FEM: ReadModel(ar); break;
        default: break;
        }
        ar.CloseChunk();
    }
}

void ModelTypeInfoReader::ReadModel(IArchive& ar)
{
    TRACE("FSModel::Load");

    // read the model data
    while (IArchive::IO_OK == ar.OpenChunk())
    {
        int nid = ar.GetChunkID();
        switch (nid)
        {
        // case CID_FEM_DATA         : LoadData(ar); break;
        // case CID_FEM_SOLUTE_DATA  : LoadSoluteData(ar); break;
        // case CID_FEM_SBM_DATA     : LoadSBMData(ar); break;
        case CID_MATERIAL_SECTION: ReadMaterials(ar); break;
        // case CID_GEOMETRY_SECTION : m_pModel->Load(ar); break;
        case CID_STEP_SECTION     : ReadSteps(ar); break;
        default: break;
        }
        ar.CloseChunk();
    }
}

void ModelTypeInfoReader::ReadMaterials(IArchive& ar)
{
    typeInfo["Material"] = unordered_set<string>();

    while (IArchive::IO_OK == ar.OpenChunk())
    {
        int ntype = ar.GetChunkID();
        typeInfo["Material"].insert(FEMaterialFactory::TypeStr(ntype));
        ar.CloseChunk();
    }
    
}

void ModelTypeInfoReader::ReadSteps(IArchive& ar)
{
    while (IArchive::IO_OK == ar.OpenChunk())
	{
		int nid = ar.GetChunkID();
		switch (nid)
		{
// 		case CID_STEP_PARAMS:
// 		{
// 			ParamContainer::Load(ar);
// 		}
// 		break;
// 		case CID_STEP_SETTINGS:
// 			{
// 				STEP_SETTINGS& o = m_ops;
// 				while (IArchive::IO_OK == ar.OpenChunk())
// 				{
// 					int nid = ar.GetChunkID();
// 					switch (nid)
// 					{
// 					case CID_STEP_TITLE     : ar.read(o.sztitle  ); break;
// 					case CID_STEP_NTIME     : ar.read(o.ntime    ); break;
// 					case CID_STEP_DT        : ar.read(o.dt       ); break;
// 					case CID_STEP_BAUTO     : ar.read(o.bauto    ); break;
// 					case CID_STEP_BMUST     : ar.read(o.bmust    ); break;
// 					case CID_STEP_MXBACK    : ar.read(o.mxback   ); break;
// 					case CID_STEP_ITEOPT    : ar.read(o.iteopt   ); break;
// 					case CID_STEP_DTMIN     : ar.read(o.dtmin    ); break;
// 					case CID_STEP_DTMAX     : ar.read(o.dtmax    ); break;
// 					case CID_STEP_MTHSOL    : ar.read(o.mthsol   ); break;
// 					case CID_STEP_BMINBW    : ar.read(o.bminbw   ); break;
// 					case CID_STEP_MATFMT    : ar.read(o.nmatfmt  ); break;
// 					case CID_STEP_ILIMIT    : ar.read(o.ilimit   ); break;
// 					case CID_STEP_MAXREF    : ar.read(o.maxref   ); break;
// 					case CID_STEP_DIVREF	: ar.read(o.bdivref  ); break;
// 					case CID_STEP_REFSTEP	: ar.read(o.brefstep ); break;
// //					case CID_STEP_DTOL      : ar.read(o.dtol     ); break;
// //					case CID_STEP_ETOL      : ar.read(o.etol     ); break;
// //					case CID_STEP_RTOL      : ar.read(o.rtol     ); break;
// //					case CID_STEP_PTOL      : ar.read(o.ptol     ); break;
// //					case CID_STEP_CTOL      : ar.read(o.ctol     ); break;
// //					case CID_STEP_VTOL      : ar.read(o.vtol     ); break;
// //					case CID_STEP_TOLLS     : ar.read(o.tolls    ); break;
// 					case CID_STEP_ANALYSIS  : ar.read(o.nanalysis); break;
// //					case CID_STEP_PSTIFFNESS: ar.read(o.bpress_stiff); break;
// //					case CID_STEP_PRES_STIFF: ar.read(o.nprstf   ); break;
// //					case CID_STEP_SYMM_PORO : ar.read(o.bsymmporo); break;
// 					case CID_STEP_CUTBACK   : ar.read(o.ncut     ); break;
// //					case CID_STEP_MINRES    : ar.read(o.minres   ); break;
// 					case CID_STEP_PLOTSTRIDE: ar.read(o.plot_stride); break;
// 					case CID_STEP_PLOTLEVEL : ar.read(o.plot_level); break;
// 					}
// 					ar.CloseChunk();

// 					// for old versions, biphasic and biphasic-solute problems
// 					// only knew transient analyses, but the analysis parameter was not 
// 					// used, so it may contain an incorrect value now. Therefore
// 					// we need to fix it.
// 					if (ar.Version() < 0x00010008)
// 					{
// 						if ((GetType() == FE_STEP_BIPHASIC) || 
// 							(GetType() == FE_STEP_BIPHASIC_SOLUTE) ||
// 							(GetType() == FE_STEP_MULTIPHASIC)) o.nanalysis = FE_DYNAMIC;
// 					}
// 				}
// 			}
// 			break;
// 		case CID_STEP_MUST_POINT: m_MP.Load(ar); break;
		case CID_STEP_DATA      : 
        {
            while (IArchive::IO_OK == ar.OpenChunk())
            {
                int ntype = ar.GetChunkID();
                switch (ntype)
                {
                case FE_STEP_INITIAL            : 
                case FE_STEP_MECHANICS          : 
                case FE_STEP_NL_DYNAMIC         : 
                case FE_STEP_HEAT_TRANSFER      : 
                case FE_STEP_BIPHASIC           : 
                case FE_STEP_BIPHASIC_SOLUTE   : 
                case FE_STEP_MULTIPHASIC		:
                case FE_STEP_FLUID              : 
                case FE_STEP_FLUID_FSI          : 
                case FE_STEP_REACTION_DIFFUSION : 
                case FE_STEP_FEBIO_ANALYSIS     : break;
                default:
                    throw ReadError("unknown CID in FSModel::LoadSteps");
                }

                TRACE("FSStep::Load");

                while (IArchive::IO_OK == ar.OpenChunk())
                {
                    int nid = ar.GetChunkID();
                    switch (nid)
                    {
                    // case CID_STEP_NAME      : { string name; ar.read(name); SetName(name); } break;
                    // case CID_FEOBJ_INFO     : { string info; ar.read(info); SetInfo(info); } break;
                    // case CID_STEP_ID        : { int nid; ar.read(nid); SetID(nid); } break;
                    // case CID_STEP_PARAMS:
                    // {
                    //     ParamContainer::Load(ar);
                    // }
                    // break;
                    // case CID_STEP_PROPERTY:
                    // {
                    //     FSStepControlProperty* pc = nullptr;
                    //     string typeString;
                    //     while (IArchive::IO_OK == ar.OpenChunk())
                    //     {
                    //         int cid = ar.GetChunkID();
                    //         switch (cid)
                    //         {
                    //         case CID_STEP_PROPERTY_NAME:
                    //         {
                    //             std::string propName;
                    //             ar.read(propName);
                    //             pc = FindControlProperty(propName); assert(pc);
                    //         }
                    //         break;
                    //         case CID_STEP_PROPERTY_TYPESTR: ar.read(typeString); break;
                    //         case CID_STEP_PROPERTY_DATA: 
                    //         {
                    //             FSStepComponent* psc = new FSStepComponent;
                    //             FEBio::CreateModelComponent(pc->m_nSuperClassId, typeString, psc);
                    //             psc->Load(ar);
                    //             assert(pc->m_prop == nullptr);
                    //             pc->m_prop = psc;
                    //         }
                    //         break;
                    //         default:
                    //             assert(false);
                    //         }
                    //         ar.CloseChunk();
                    //     }
                    // }
                    // break;
                    case CID_BC_SECTION: // boundary conditions
                    {
                        typeInfo["Boundary"] = unordered_set<string>();

                        while (IArchive::IO_OK == ar.OpenChunk())
                        {
                            int ntype = ar.GetChunkID();
                            
                            FEMKernel* kernel = FEMKernel::Instance();
                            typeInfo["Boundary"].insert(kernel->TypeStr(FEBC_ID, ntype));




                            

                            // FSDomainComponent* pb = 0;
                            // switch (ntype)
                            // {
                            // case FE_FIXED_DISPLACEMENT		 : pb = new FSFixedDisplacement         (nullptr); break;
                            // case FE_FIXED_ROTATION           : pb = new FSFixedRotation             (nullptr); break;
                            // case FE_FIXED_FLUID_PRESSURE	 : pb = new FSFixedFluidPressure        (nullptr); break;
                            // case FE_FIXED_TEMPERATURE        : pb = new FSFixedTemperature          (nullptr); break;
                            // case FE_FIXED_CONCENTRATION      : pb = new FSFixedConcentration        (nullptr); break;
                            // case FE_FIXED_FLUID_VELOCITY     : pb = new FSFixedFluidVelocity        (nullptr); break;
                            // case FE_FIXED_DILATATION         : pb = new FSFixedFluidDilatation      (nullptr); break;
                            // case FE_PRESCRIBED_DISPLACEMENT	 : pb = new FSPrescribedDisplacement    (nullptr); break;
                            // case FE_PRESCRIBED_ROTATION      : pb = new FSPrescribedRotation        (nullptr); break;
                            // case FE_PRESCRIBED_FLUID_PRESSURE: pb = new FSPrescribedFluidPressure   (nullptr); break;
                            // case FE_PRESCRIBED_TEMPERATURE   : pb = new FSPrescribedTemperature     (nullptr); break;
                            // case FE_PRESCRIBED_CONCENTRATION : pb = new FSPrescribedConcentration   (nullptr); break;
                            // case FE_PRESCRIBED_FLUID_VELOCITY: pb = new FSPrescribedFluidVelocity   (nullptr); break;
                            // case FE_PRESCRIBED_DILATATION    : pb = new FSPrescribedFluidDilatation (nullptr); break;
                            // case FE_FIXED_SHELL_DISPLACEMENT : pb = new FSFixedShellDisplacement    (nullptr); break;
                            // case FE_PRESCRIBED_SHELL_DISPLACEMENT: pb = new FSPrescribedShellDisplacement(nullptr); break;
                            // case FE_FEBIO_BC                 : pb = new FEBioBoundaryCondition(nullptr); break;
                            // default:
                            //     if (ar.Version() < 0x00020000)
                            //     {
                            //         // initial conditions used to be grouped with the boundary conditions, but
                            //         // now have their own section
                            //         switch(ntype)
                            //         {
                            //         case FE_NODAL_VELOCITIES		 : pb = new FSNodalVelocities       (nullptr); break;
                            //         case FE_NODAL_SHELL_VELOCITIES	 : pb = new FSNodalShellVelocities  (nullptr); break;
                            //         case FE_INIT_FLUID_PRESSURE      : pb = new FSInitFluidPressure     (nullptr); break;
                            //         case FE_INIT_SHELL_FLUID_PRESSURE: pb = new FSInitShellFluidPressure(nullptr); break;
                            //         case FE_INIT_CONCENTRATION       : pb = new FSInitConcentration     (nullptr); break;
                            //         case FE_INIT_SHELL_CONCENTRATION : pb = new FSInitShellConcentration(nullptr); break;
                            //         case FE_INIT_TEMPERATURE         : pb = new FSInitTemperature       (nullptr); break;
                            //         default:
                            //             throw ReadError("error parsing CID_BC_SECTION in FSStep::Load");
                            //         }
                            //     }
                            //     else
                            //     {
                            //         throw ReadError("error parsing CID_BC_SECTION in FSStep::Load");
                            //     }
                            // }

                            // pb->Load(ar);

                            // typeInfo["Boundary"].insert(pb->GetTypeString());

                            // delete pb;

                            // if (ar.Version() < 0x0001000F)
                            // {
                            //     // In older version, the rotational dofs were actually stored in the FSFixedDisplacement
                            //     // but now, they are stored in FSFixedRotation
                            //     FSFixedDisplacement* pbc = dynamic_cast<FSFixedDisplacement*>(pb);
                            //     if (pbc)
                            //     {
                            //         int bc = pbc->GetBC();
                            //         if (bc >= 8)
                            //         {
                            //             bc = (bc>>3)&7;
                            //             FSFixedRotation* prc = new FSFixedRotation(m_pfem);
                            //             prc->SetName(pbc->GetName());
                            //             prc->SetBC(bc);
                            //             prc->SetItemList(pb->GetItemList()->Copy());
                            //             AddBC(prc);
                            //         }
                            //     }

                            //     FSPrescribedDisplacement* pdc = dynamic_cast<FSPrescribedDisplacement*>(pb);
                            //     if (pdc)
                            //     {
                            //         int bc = pdc->GetDOF();
                            //         if (bc > 2)
                            //         {
                            //             bc -= 3;
                            //             FSPrescribedRotation* prc = new FSPrescribedRotation(m_pfem);
                            //             prc->SetName(pdc->GetName());
                            //             prc->SetDOF(bc);
                            //             prc->SetItemList(pdc->GetItemList()->Copy());
                            //             delete pdc;
                            //             pb = prc;
                            //         }
                            //     }
                            // }

                            // // add BC
                            // if (ar.Version() < 0x00020000)
                            // {
                            //     if (dynamic_cast<FSInitialCondition*>(pb)) AddIC(dynamic_cast<FSInitialCondition*>(pb));
                            //     else AddBC(dynamic_cast<FSBoundaryCondition*>(pb));
                            // }
                            // else AddBC(dynamic_cast<FSBoundaryCondition*>(pb));

                            ar.CloseChunk();
                        }
                    }
                    break;
                    // case CID_FC_SECTION: // loads
                    //     {
                    //         while (IArchive::IO_OK == ar.OpenChunk())
                    //         {
                    //             int ntype = ar.GetChunkID();

                    //             FSLoad* pl = 0;

                    //             if (ntype == FE_NODAL_DOF_LOAD) pl = new FSNodalDOFLoad(m_pfem);
                    //             else if (ntype == FE_FEBIO_NODAL_LOAD) pl = new FEBioNodalLoad(m_pfem);
                    //             else
                    //             {
                    //                 // see if it's a surface load
                    //                 pl = fecore_new<FSLoad>(m_pfem, FESURFACELOAD_ID, ntype);
                    //                 if (pl == 0)
                    //                 {
                    //                     // could be a body load
                    //                     pl = fecore_new<FSLoad>(m_pfem, FEBODYLOAD_ID, ntype);
                    //                 }
                    //             }

                    //             // make sure we have something
                    //             if (pl == 0)
                    //             {
                    //                 throw ReadError("error parsing CID_FC_SECTION FSStep::Load");
                    //             }

                    //             // read BC data
                    //             pl->Load(ar);

                    //             // add BC
                    //             AddLoad(pl);

                    //             ar.CloseChunk();
                    //         }
                    //     }
                    //     break;
                    // case CID_IC_SECTION: // initial conditions
                    //     {
                    //         while (IArchive::IO_OK == ar.OpenChunk())
                    //         {
                    //             int ntype = ar.GetChunkID();

                    //             FSInitialCondition* pi = 0;
                    //             switch (ntype)
                    //             {
                    //             case FE_INIT_FLUID_PRESSURE      : pi = new FSInitFluidPressure     (m_pfem); break;
                    //             case FE_INIT_SHELL_FLUID_PRESSURE: pi = new FSInitShellFluidPressure(m_pfem); break;
                    //             case FE_INIT_CONCENTRATION       : pi = new FSInitConcentration     (m_pfem); break;
                    //             case FE_INIT_SHELL_CONCENTRATION : pi = new FSInitShellConcentration(m_pfem); break;
                    //             case FE_INIT_TEMPERATURE         : pi = new FSInitTemperature       (m_pfem); break;
                    //             case FE_NODAL_VELOCITIES         : pi = new FSNodalVelocities       (m_pfem); break;
                    //             case FE_NODAL_SHELL_VELOCITIES   : pi = new FSNodalShellVelocities  (m_pfem); break;
                    //             case FE_INIT_FLUID_DILATATION    : pi = new FSInitFluidDilatation   (m_pfem); break;
                    //             case FE_INIT_PRESTRAIN           : pi = new FSInitPrestrain         (m_pfem); break;
                    //             case FE_FEBIO_INITIAL_CONDITION  : pi = new FEBioInitialCondition   (m_pfem); break;
                    //             default:
                    //                 throw ReadError("error parsing CID_IC_SECTION FSStep::Load");
                    //             }

                    //             // read data
                    //             pi->Load(ar);

                    //             // Add IC
                    //             AddIC(pi);

                    //             ar.CloseChunk();
                    //         }
                    //     }
                    //     break;
                    // case CID_INTERFACE_SECTION: // interfaces
                    //     {
                    //         while (IArchive::IO_OK == ar.OpenChunk())
                    //         {
                    //             int ntype = ar.GetChunkID();

                    //             FSInterface* pi = 0;

                    //             // check obsolete interfaces first
                    //             if      (ntype == FE_SLIDING_INTERFACE   ) pi = new FSSlidingInterface(m_pfem);
                    //             else if (ntype == FE_SPRINGTIED_INTERFACE) pi = new FSSpringTiedInterface(m_pfem);
                    //             else pi = fecore_new<FSInterface>(m_pfem, FESURFACEINTERFACE_ID, ntype);

                    //             // make sure we were able to allocate an interface
                    //             if (pi == 0)
                    //             {
                    //                 // some "contact" interfaces were moved to constraints
                    //                 FSModelConstraint* pc = fecore_new<FSModelConstraint>(m_pfem, FENLCONSTRAINT_ID, ntype);
                    //                 if (pc)
                    //                 {
                    //                     pc->Load(ar);
                    //                     AddConstraint(pc);
                    //                 }
                    //                 else throw ReadError("error parsing unknown CID_INTERFACE_SECTION FSStep::Load");
                    //             }
                    //             else
                    //             {
                    //                 // load the interface data
                    //                 pi->Load(ar);

                    //                 // add interface to step
                    //                 AddInterface(pi);
                    //             }

                    //             ar.CloseChunk();
                    //         }
                    //     }
                    //     break;
                    // case CID_CONSTRAINT_SECTION: // model constraints
                    // {
                    //     while (IArchive::IO_OK == ar.OpenChunk())
                    //     {
                    //         int ntype = ar.GetChunkID();

                    //         FSModelConstraint* pmc = fecore_new<FSModelConstraint>(m_pfem, FENLCONSTRAINT_ID, ntype);

                    //         // make sure we were able to allocate a constraint
                    //         if (pmc == 0)
                    //         {
                    //             throw ReadError("error parsing unknown CID_INTERFACE_SECTION FSStep::Load");
                    //         }
                    //         else
                    //         {
                    //             // load the constraint data
                    //             pmc->Load(ar);

                    //             // add constraint to step
                    //             AddConstraint(pmc);
                    //         }

                    //         ar.CloseChunk();
                    //     }
                    // }
                    // break;
                    // case CID_RC_SECTION: // rigid constraints
                    //     {
                    //         while (IArchive::IO_OK == ar.OpenChunk())
                    //         {
                    //             int ntype = ar.GetChunkID();

                    //             if (ar.Version() < 0x00020000)
                    //             {
                    //                 FSRigidConstraintOld* rc_old = new FSRigidConstraintOld(ntype, GetID());

                    //                 // read RC data
                    //                 rc_old->Load(ar);

                    //                 vector<FSRigidConstraint*> rc = convertOldToNewRigidConstraint(m_pfem, rc_old);

                    //                 // add rigid constraints
                    //                 for (int i=0; i<(int) rc.size(); ++i) AddRC(rc[i]);
                    //             }
                    //             else
                    //             {
                    //                 FSRigidConstraint* rc = 0;
                    //                 switch (ntype)
                    //                 {
                    //                 case FE_RIGID_FIXED            : rc = new FSRigidFixed          (m_pfem, GetID()); break;
                    //                 case FE_RIGID_DISPLACEMENT     : rc = new FSRigidDisplacement   (m_pfem, GetID()); break;
                    //                 case FE_RIGID_FORCE            : rc = new FSRigidForce          (m_pfem, GetID()); break;
                    //                 case FE_RIGID_INIT_VELOCITY    : rc = new FSRigidVelocity       (m_pfem, GetID()); break;
                    //                 case FE_RIGID_INIT_ANG_VELOCITY: rc = new FSRigidAngularVelocity(m_pfem, GetID()); break;
                    //                 case FE_FEBIO_RIGID_CONSTRAINT : rc = new FEBioRigidConstraint  (m_pfem, GetID()); break;
                    //                 default:
                    //                     assert(false);
                    //                 }

                    //                 if (rc)
                    //                 {
                    //                     rc->Load(ar);
                    //                     AddRC(rc);
                    //                 }
                    //             }

                    //             ar.CloseChunk();
                    //         }
                    //     }
                    //     break;
                    // case CID_CONNECTOR_SECTION: // connectors
                    //     {
                    //         while (IArchive::IO_OK == ar.OpenChunk())
                    //         {
                    //             int ntype = ar.GetChunkID();
                                
                    //             FSRigidConnector* pi = 0;
                    //             switch (ntype)
                    //             {
                    //                 case FE_RC_SPHERICAL_JOINT		: pi = new FSRigidSphericalJoint    (m_pfem); break;
                    //                 case FE_RC_REVOLUTE_JOINT		: pi = new FSRigidRevoluteJoint     (m_pfem); break;
                    //                 case FE_RC_PRISMATIC_JOINT		: pi = new FSRigidPrismaticJoint    (m_pfem); break;
                    //                 case FE_RC_CYLINDRICAL_JOINT	: pi = new FSRigidCylindricalJoint  (m_pfem); break;
                    //                 case FE_RC_PLANAR_JOINT         : pi = new FSRigidPlanarJoint       (m_pfem); break;
                    //                 case FE_RC_RIGID_LOCK           : pi = new FSRigidLock              (m_pfem); break;
                    //                 case FE_RC_SPRING               : pi = new FSRigidSpring            (m_pfem); break;
                    //                 case FE_RC_DAMPER               : pi = new FSRigidDamper            (m_pfem); break;
                    //                 case FE_RC_ANGULAR_DAMPER       : pi = new FSRigidAngularDamper     (m_pfem); break;
                    //                 case FE_RC_CONTRACTILE_FORCE    : pi = new FSRigidContractileForce  (m_pfem); break;
                    //                 case FE_RC_GENERIC_JOINT        : pi = new FSGenericRigidJoint      (m_pfem); break;
                    //                 case FE_FEBIO_RIGID_CONNECTOR   : pi = new FEBioRigidConnector      (m_pfem); break;
                    //                 default:
                    //                     throw ReadError("error parsing unknown CID_CONNECTOR_SECTION FSStep::Load");
                    //             }
                                
                    //             // load the interface data
                    //             pi->Load(ar);
                                
                    //             // add interface to step
                    //             AddRigidConnector(pi);
                                
                    //             ar.CloseChunk();
                    //         }
                    //     }
                    //     break;
                    default: break;
                    }
                    ar.CloseChunk();
                }

                ar.CloseChunk();
            }
        }
        default: break;
		}
		ar.CloseChunk();
	}	





}