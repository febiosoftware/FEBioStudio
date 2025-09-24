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

// NIKE3DExport.cpp: implementation of the NIKE3DExport class.
//
//////////////////////////////////////////////////////////////////////

#include "NIKE3DExport.h"
#include <FEMLib/FSProject.h>
using namespace std;

//-----------------------------------------------------------------------------
// NIKE3DExport
//-----------------------------------------------------------------------------

NIKE3DExport::NIKE3DExport(FSProject& prj) : FSFileExport(prj)
{
	m_bcomm = false;
	m_fp = nullptr;
}

NIKE3DExport::~NIKE3DExport()
{

}

//-----------------------------------------------------------------------------

bool NIKE3DExport::Write(const char* szfile)
{
	// try to create a NIKE project
	NIKE3DProject nike; 
	if (nike.Create(m_prj) == false) return errf("Failed creating NIKE project.");

	// open the file
	FILE* fp = m_fp = fopen(szfile, "wt");
	if (fp == 0) return false;

	m_bcomm = true;

	// ===== C O N T R O L   D E C K =====
	if (!ExportControl(nike)) return false;

	// ===== M A T E R I A L   D E C K =====
	if (!ExportMaterials(nike)) return false;

	// ===== N O D E   P O I N T   D E C K =====
	if (!ExportNodes(nike)) return false;

	// ===== H E X A H E D R O N   E L E M E N T   D E C K =====
	if (!ExportHexElements(nike)) return false;

	// ===== S H E L L   E L E M E N T   D E C K =====
	if (!ExportShellElements(nike)) return false;

	// ===== R I G I D   N O D E   A N D   F A C E T   D E C K =====
	if (!ExportRigid(nike)) return false;

	// ===== D I S C R E T E   E L E M E N T   D E C K =====
	if (!ExportDiscrete(nike)) return false;

	// ===== S L I D I N G   S U R F A C E   D E C K =====
	if (!ExportSliding(nike)) return false;

	// ===== L O A D   C U R V E   D E C K =====
	if (!ExportLoadCurve(nike)) return false;

	// ===== C O N C E N T R A T E D   N O D A L   L O A D S   D E C K =====
	if (!ExportNodalLoads(nike)) return false;

	// ===== P R E S S U R E   B O U N D A R Y   C O N D I T I O N   D E C K =====
	if (!ExportPressure(nike)) return false;

	// ===== D I S P L A C E M E N T   B O U N D A R Y   C O N D I T I O N   D E C K =====
	if (!ExportDisplacement(nike)) return false;

	// ===== B A S E   A C C E L E R A T I O N   B O D Y   F O R C E   D E C K =====
	if (!ExportBodyForce(nike)) return false;

	// ===== A N G U L A R   V E L O C I T Y   B O D Y   F O R C E   D E C K =====
	// TODO : export angular velocity body force deck

	// ===== I N I T I A L   V E L O C I T Y   D E C K =====
	if (!ExportVelocities(nike)) return false;

	// ===== F O U N D A T I O N   N O D E   D E C K =====
	// TODO : export foundation node boundary condition deck

	fclose(fp);
	fp = m_fp = 0;
	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportControl(NIKE3DProject& prj)
{
	char	szauto[6];
	char	sztogg[6];
	int		n[15];
	double	f[15];

	NIKE3DProject::CONTROL& c = prj.m_Ctrl;

	if (m_bcomm)
	{
		fprintf(m_fp,"*************************************************************************************\n");
		fprintf(m_fp,"** NIKE3D input deck, created by :                                                 **\n");
		fprintf(m_fp,"**                                                                                 **\n");
		fprintf(m_fp,"**                              P R E V I E W   1.0                                **\n");
		fprintf(m_fp,"**                                                                                 **\n");
		fprintf(m_fp,"*************************************************************************************\n");
	}
	fprintf(m_fp, "%-40s\n", c.sztitle);

	// control card 2 - Input settings
	if (m_bcomm)
	{
		fprintf(m_fp,"********************************** CONTROL CARD  2 **********************************\n");
		fprintf(m_fp,"* [ 1] input format\n");
		fprintf(m_fp,"* [ 2] number of materials\n");
		fprintf(m_fp,"* [ 3] number of nodal points\n");
		fprintf(m_fp,"* [ 4] number of brick elements\n");
		fprintf(m_fp,"* [ 5] number of beam elements\n");
		fprintf(m_fp,"* [ 6] number of shell elements\n");
		fprintf(m_fp,"* [ 7] number of 1D slide lines\n");
		fprintf(m_fp,"* [ 8] number of sliding surfaces\n");
		fprintf(m_fp,"* [ 9] number of rigid walls and symmetry plances\n");
		fprintf(m_fp,"* [10] discrete element input flag\n");
		fprintf(m_fp,"* [11] number of rigid nodes and facets\n");
		fprintf(m_fp,"*|-2|----3----|----4----|----5----|----6----|--7-|--8-|--9-|-10-|-11-|\n");
	}
	n[0] = c.nmmat;
	n[1] = c.numnp;
	n[2] = c.numelh;
	n[3] = c.numelb;
	n[4] = c.numels;
	n[5] = c.num1d;
	n[6] = c.numsi;
	n[7] = c.nrwsp;
	n[8] = c.inpde;
	n[9] = c.numrnf;
	n[10] = (prj.m_DSP.size() > 0? 1 : 0);

	fprintf(m_fp,"FL%3d%10d%10d%10d%10d%5d%5d%5d%5d%5d%5d\n",n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],n[8],n[9],n[10]);

	// control card 3 - Time settings
	if (m_bcomm)
	{
		fprintf(m_fp,"********************************** CONTROL CARD  3 **********************************\n");
		fprintf(m_fp,"* [ 1] number of time or load steps\n");
		fprintf(m_fp,"* [ 2] time or load step size\n");
		fprintf(m_fp,"* [ 3] auto time/load stap control flag\n");
		fprintf(m_fp,"* [ 4] maximum number of retries allowable per step\n");
		fprintf(m_fp,"* [ 5] optimal number of iterations per step\n");
		fprintf(m_fp,"* [ 6] minimal allowable time/load step size\n");
		fprintf(m_fp,"* [ 7] maximum allowable time/load step size\n");
		fprintf(m_fp,"* [ 8] size of optimal iteration window (RATL only)\n");
		fprintf(m_fp,"* [ 9] superplastic forming flag\n");
		fprintf(m_fp,"* [10] brick analysis flag\n");
		fprintf(m_fp,"* [11] shell analysis flag\n");
		fprintf(m_fp,"* [12] beam analysis  flag\n");
		fprintf(m_fp,"* [13] pressure analysis flag\n");
		fprintf(m_fp,"*----1---|----2----|--3-|--4-|--5-|----6----|----7----|--8-|-9|-10-|-11-|-12-|-13-|\n");
	}
	n[0] = c.ntime;
	f[1] = c.dt;
	switch (c.nauto)
	{
	case 1: strcpy(szauto, " auto"); break;
	case 2: strcpy(szauto, " autv"); break;
	case 3: strcpy(szauto, " ratl"); break;
	case 4: strcpy(szauto, " cost"); break;
	case 5: strcpy(szauto, " load"); break;
	case 6: strcpy(szauto, "uload"); break;
	default:
		strcpy(szauto, "     ");
	}
	n[3] = c.mxback;
	n[4] = c.iteopt;
	f[5] = c.dtmin;
	f[6] = c.dtmax;
	n[7] = c.irfwin;
	n[8] = c.spf;
	n[9] = c.linearb;
	n[10] = c.linears;
	n[11] = c.linearbm;
	n[12] = c.linearpr;
	fprintf(m_fp,"%10d%10.3g%5s%5d%5d%10.3g%10.3g%5d%3d%5d%5d%5d%5d\n",n[0],f[1],szauto,n[3],n[4],f[5],f[6],n[7],n[8],n[9],n[10],n[11],n[12]);

	// control card 4 - Boundary conditions
	if (m_bcomm)
	{
		fprintf(m_fp,"********************************** CONTROL CARD  4 **********************************\n");
		fprintf(m_fp,"* [ 1] number of load curves\n");
		fprintf(m_fp,"* [ 2] maximum number of points defining any load curve\n");
		fprintf(m_fp,"* [ 3] number of concentrated nodal loads\n");
		fprintf(m_fp,"* [ 4] number of element surfaces having pressure loadings applied\n");
		fprintf(m_fp,"* [ 5] number of displacement boundary condition cards\n");
		fprintf(m_fp,"* [ 6] number of beam elements with aerodynamic drag loads\n");
		fprintf(m_fp,"* [ 7] number of node constraint cards\n");
		fprintf(m_fp,"* [ 8] body force loads due to base-acceleration in the x-direction\n");
		fprintf(m_fp,"* [ 9] body force loads due to base-acceleration in the y-direction\n");
		fprintf(m_fp,"* [10] body force loads due to base-acceleration in the z-direction\n");
		fprintf(m_fp,"* [11] body force loads due to angular velocity about x-axis\n");
		fprintf(m_fp,"* [12] body force loads due to angular velocity about y-axis\n");
		fprintf(m_fp,"* [13] body force loads due to angular velocity about z-axis\n");
		fprintf(m_fp,"* [14] number of nodes with steering boundary conditions\n");
		fprintf(m_fp,"* [15] number of nodes with foundation boundary conditions\n");
		fprintf(m_fp,"*-1-|--2-|--3-|--4-|--5-|--6-|--7-|--8-|--9-|-10-|-11-|-12-|-13-|-14-|-15-|\n");
	}
	n[0] = c.numlc;
	n[1] = c.nptm;
	n[2] = c.numcnl;
	n[3] = c.numpr;
	n[4] = c.numdis;
	n[5] = c.numadl;
	n[6] = c.nrcc;
	n[7] = c.bfa[0];
	n[8] = c.bfa[1];
	n[9] = c.bfa[2];
	n[10] = c.bfav[0];
	n[11] = c.bfav[1];
	n[12] = c.bfav[2];
	n[13] = c.nsteer;
	n[14] = c.nfnbc;
	fprintf(m_fp,"%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d%5d\n",n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],n[8],n[9],n[10],n[11],n[12],n[13],n[14]);

	// control card 5 - output settings
	if (m_bcomm)
	{
		fprintf(m_fp,"********************************** CONTROL CARD  5 **********************************\n");
		fprintf(m_fp,"* [ 1] output printing interval\n");
		fprintf(m_fp,"* [ 2] output plotting interval\n");
		fprintf(m_fp,"* [ 3] number of node print blocks\n");
		fprintf(m_fp,"* [ 4] number of brick element print blocks\n");
		fprintf(m_fp,"* [ 5] number of beam element print blocks\n");
		fprintf(m_fp,"* [ 6] number of shell element print blocks\n");
		fprintf(m_fp,"* [ 7] number of time steps between running restart file generation\n");
		fprintf(m_fp,"* [ 8] number of time steps between restart file generation\n");
		fprintf(m_fp,"* [ 9] shell element strain dump flag\n");
		fprintf(m_fp,"* [10] initial sense switch toggles\n");
		fprintf(m_fp,"* [11] acceleration data dump flag\n");
		fprintf(m_fp,"* [12] maximum augmented lagrangian iterations per step\n");
		fprintf(m_fp,"* [13] interface gap plot flag\n");
		fprintf(m_fp,"*-1-|--2-|--3-|--4-|--5-|--6-|--7-|--8-|--9-|-10-|-11-|-12-|-13-|\n");
	}
	n[0] = c.ipri;
	n[1] = c.jpri;
	n[2] = c.nnpb;
	n[3] = c.nhpb;
	n[4] = c.nbpb;
	n[5] = c.nspb;
	n[6] = c.jrfreq;
	n[7] = c.irfreq;
	n[8] = c.istrn;
	sprintf(sztogg, "%1d%1d%1d%1d%1d", c.sw[0], c.sw[1], c.sw[2], c.sw[3], c.sw[4]);
	n[10] = c.iacflg;
	n[11] = c.maxaug;
	n[12] = c.ingap;
	fprintf(m_fp,"%5d%5d%5d%5d%5d%5d%5d%5d%5d%5s%5d%5d%5d\n",n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],n[8],sztogg,n[10],n[11],n[12]);

	// control card 6 - solver settings
	if (m_bcomm)
	{
		fprintf(m_fp,"********************************** CONTROL CARD  6 **********************************\n");
		fprintf(m_fp,"* [ 1] nonlinear equilibrium solution setting\n");
		fprintf(m_fp,"* [ 2] solver specific flags (see n3dhsp for options)\n");
		fprintf(m_fp,"* [ 3] number of steps between stiffness reformation at beginning of step\n");
		fprintf(m_fp,"* [ 4] number of time steps between equilibrium iterations\n");
		fprintf(m_fp,"* [ 5] maximum number of quasi-newton equilibrium iterations permitted\n");
		fprintf(m_fp,"*      between stiffness matrix reformations\n");
		fprintf(m_fp,"* [ 6] maximum number of stiffness matrix reformations per time step\n");
		fprintf(m_fp,"* [ 7] convergence tolerance on displacements\n");
		fprintf(m_fp,"* [ 8] convergence tolerance on energy\n");
		fprintf(m_fp,"* [ 9] convergence tolerance on residual\n");
		fprintf(m_fp,"* [10] convergence tolerance on line search\n");
		fprintf(m_fp,"*-1-|--2-|----3----|----4----|--5-|--6-|----7----|----8----|----9----|---10----|\n");
	}
	n[0] = c.mthsol;
	n[1] = c.sflag;
	n[2] = c.icnt1;
	n[3] = c.icnt2;
	n[4] = c.ilimit;
	n[5] = c.maxref;
	f[6] = c.dtol;
	f[7] = c.ectl;
	f[8] = c.rctl;
	f[9] = c.tolls;
	fprintf(m_fp,"%5d%5d%10d%10d%5d%5d%10.3g%10.3g%10.3g%10.3g\n",n[0],n[1],n[2],n[3],n[4],n[5],f[6],f[7],f[8],f[9]);

	// control card 7 - analysis settings
	if (m_bcomm)
	{
		fprintf(m_fp,"********************************** CONTROL CARD  7 **********************************\n");
		fprintf(m_fp,"* [ 1] analysis type\n");
		fprintf(m_fp,"* [ 2] initial velocity flag\n");
		fprintf(m_fp,"* [ 3] thermal effects option\n");
		fprintf(m_fp,"* [ 4] temperature profile input flag\n");
		fprintf(m_fp,"* [ 5] number of eigenvalues and eigenvectors to be extracted\n");
		fprintf(m_fp,"* [ 6] frequency shift, cycles per unit time\n");
		fprintf(m_fp,"* [ 7] first Newmark integration parameter\n");
		fprintf(m_fp,"* [ 8] second Newmark integration parameter\n");
		fprintf(m_fp,"* [ 9] Hilber Hughes Taylor HHT integration parameter\n");
		fprintf(m_fp,"* [10] eigenvalue solver convergence tolerance\n");
		fprintf(m_fp,"* [11] eigensolver maximum iterations number\n");
		fprintf(m_fp,"* [12] eigensolution method\n");
		fprintf(m_fp,"*-1-|--2-|--3-|--4-|--5-|----6----|----7----|----8----|----9----|----10---|-11-|-12-|\n");
	}
	n[0] = c.imass;
	n[1] = c.intvel;
	n[2] = c.iteo;
	n[3] = c.itpro;
	n[4] = c.neig;
	f[5] = c.eshift;
	f[6] = c.fnip;
	f[7] = c.snip;
	f[8] = c.alpha;
	f[9] = c.etol;
	n[10] = c.ieigit;
	n[11] = c.ieigens;
	fprintf(m_fp,"%5d%5d%5d%5d%5d%10.3g%10.3g%10.3g%10.3g%10.3g%5d%5d\n",n[0],n[1],n[2],n[3],n[4],f[5],f[6],f[7],f[8],f[9],n[10],n[11]);

	// control card 8 - formulation settings
	if (m_bcomm)
	{
		fprintf(m_fp,"********************************** CONTROL CARD  8 **********************************\n");
		fprintf(m_fp,"* [ 1] symmetric/unsymmetric stiffness flag\n");
		fprintf(m_fp,"* [ 2] element data buffer size\n");
		fprintf(m_fp,"* [ 3] direct linear equation solver\n");
		fprintf(m_fp,"* [ 4] BFGS update vector storage option\n");
		fprintf(m_fp,"* [ 5] brick element formulation\n");
		fprintf(m_fp,"* [ 6] brick element geometric stiffness flag\n");
		fprintf(m_fp,"* [ 7] shell element formulation\n");
		fprintf(m_fp,"* [ 8] hourglass control parameter (B-T shell only)\n");
		fprintf(m_fp,"* [ 9] shell element geometric stiffness flag\n");
		fprintf(m_fp,"* [10] beam element formulation\n");
		fprintf(m_fp,"* [11] beam element geometric stiffness flag\n");
		fprintf(m_fp,"* [12] contact search and linearization flag\n");
		fprintf(m_fp,"* [13] rotary inertia flag\n");
		fprintf(m_fp,"* [14] geometric stiffness initiation treshold\n");
		fprintf(m_fp,"*-1-|----2----|--3-|--4-|--5-|--6-|--7-|----8----|--9-|-10-|-11-|-12-|-13-|-14-|\n");
	}
	n[0] = c.iunsym;
	n[1] = c.nwebin;
	n[2] = c.ifiss;
	n[3] = c.ioroin;
	n[4] = c.ibrfor;
	n[5] = c.igsoin;
	n[6] = c.ishfin;
	f[7] = c.qhg;
	n[8] = c.isgsin;
	n[9] = c.ibmfin;
	n[10] = c.ibgsin;
	n[11] = c.istold;
	n[12] = c.irotary;
	n[13] = c.geotol;
	fprintf(m_fp,"%5d%10d%5d%5d%5d%5d%5d%10.3g%5d%5d%5d%5d%5d%5d\n",n[0],n[1],n[2],n[3],n[4],n[5],n[6],f[7],n[8],n[9],n[10],n[11],n[12],n[13]);

	// control card 9 - integration settings
	if (m_bcomm)
	{
		fprintf(m_fp,"********************************** CONTROL CARD  9 **********************************\n");
		fprintf(m_fp,"* [ 1] (not used)\n");
		fprintf(m_fp,"* [ 2] desired initial arc length\n");
		fprintf(m_fp,"* [ 3] arc length constraint method\n");
		fprintf(m_fp,"* [ 4] (not used)\n");
		fprintf(m_fp,"* [ 5] number of user-specified integration rules for beams\n");
		fprintf(m_fp,"* [ 6] maximum number of user-specified beam integration points\n");
		fprintf(m_fp,"* [ 7] number of user-specified integration rules for shells\n");
		fprintf(m_fp,"* [ 8] maximum number of user-specified shell integration rules\n");
		fprintf(m_fp,"*---------1--------|----2----|--3-|--4-|--5-|--6-|--7-|--8-|\n");
	}
	n[0] = 0;
	f[1] = c.dsx;
	n[2] = c.irco;
	n[3] = 0;
	n[4] = c.nusbir;
	n[5] = c.mpubr;
	n[6] = c.nussir;
	n[7] = c.mpusr;
	fprintf(m_fp,"%20d%10.3g%5d%5d%5d%5d%5d%5d\n",n[0],f[1],n[2],n[3],n[4],n[5],n[6],n[7]);

	// control card 10 - iterative solver settings
	if (m_bcomm)
	{
		fprintf(m_fp,"********************************** CONTROL CARD 10 **********************************\n");
		fprintf(m_fp,"* [ 1] iterative linear equation solver option\n");
		fprintf(m_fp,"* [ 2] iteration limit for linear solver\n");
		fprintf(m_fp,"* [ 3] iteration convergence tolerance\n");
		fprintf(m_fp,"* [ 4] buffer size (elements) for out-of-core iterative linear solver\n");
		fprintf(m_fp,"* [ 5] print-out option for linear iterative solver\n");
		fprintf(m_fp,"* [ 6] iterative solver data storage option\n");
		fprintf(m_fp,"*-1-|--2-|----3----|--4-|--5-|--6-|\n");
	}
	n[0] = c.itrsol;
	n[1] = c.itrlmt;
	f[2] = c.tollin;
	n[3] = c.lbuf;
	n[4] = c.itrpnt;
	n[5] = c.iebeoc;
	fprintf(m_fp,"%5d%5d%10.3g%5d%5d%5d\n",n[0],n[1],f[2],n[3],n[4],n[5]);

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportMaterials(NIKE3DProject& prj)
{
	int nmat = (int) prj.m_Mat.size();
	if (nmat == 0) return true;

	if (m_bcomm)
	{
		fprintf(m_fp,"***************************** M A T E R I A L   D E C K *****************************\n");
	}

	double* d;
	for (int i=0; i<nmat; ++i)
	{
		NIKE3DProject::MATERIAL& m = prj.m_Mat[i];

		fprintf(m_fp,"%5d%5d%10.3g%5d%10.3g%10.3g%10.3g%10.3g%10.3g\n", i+1, m.ntype, m.dens, m.nelem, m.Tref, m.rda, m.rdb, m.hrgl, m.flag);
		fprintf(m_fp, "%s\n", m.szname);

		for (int j=0; j<6; j++)
		{
			d = m.m[j];
			fprintf(m_fp,"%10.3g%10.3g%10.3g%10.3g%10.3g%10.3g%10.3g%10.3g\n",d[0],d[1],d[2],d[3],d[4],d[5],d[6],d[7]);
		}

		if (m.nelem == 2)
		{
			d = m.m[6]; fprintf(m_fp, "%10.3g%10.3g%10.3g%10.3g%10.3g%10.3g%10.3g%10.3g\n", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);
			d = m.m[7]; fprintf(m_fp, "%10.3g%10.3g%10.3g%10.3g%10.3g%10.3g%10.3g%10.3g\n", d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportNodes(NIKE3DProject& prj)
{
	// make sure there are any nodes
	int nodes = (int)prj.m_Node.size();
	if (nodes == 0) return true;

	// print the comment
	if (m_bcomm)
	{
		fprintf(m_fp,"************************** N O D A L   P O I N T   D E C K **************************\n");
	}

	// write nodal data
	int n[7];
	double f[5];
	for (int i=0; i<nodes; ++i)
	{
		NIKE3DProject::NODE& node = prj.m_Node[i];

		n[0] = i+1;
		n[1] = node.bc;
		f[2] = node.x;
		f[3] = node.y;
		f[4] = node.z;
		n[5] = node.rc;

		fprintf(m_fp,"%8d%5d%#20.13g%#20.13g%#20.13g%5d\n",n[0],n[1],f[2],f[3],f[4],n[5]);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportHexElements(NIKE3DProject& prj)
{
	int solids = (int)prj.m_Brick.size();
	if (solids == 0) return true;

	if (m_bcomm)
	{
		fprintf(m_fp,"******************* H E X A H E D R O N   E L E M E N T   D E C K *******************\n");
	}

	int n[20];
	for (int i=0; i<solids; ++i)
	{
		NIKE3DProject::BRICK& b = prj.m_Brick[i];
		n[0] = i+1;
		n[1] = b.nmat+1;
		n[2] = b.n[0];
		n[3] = b.n[1];
		n[4] = b.n[2];
		n[5] = b.n[3];
		n[6] = b.n[4];
		n[7] = b.n[5];
		n[8] = b.n[6];
		n[9] = b.n[7];

		if (b.nnew)
		{
			// currently only supported for tet10 elements
			n[10] = b.n[8];
			n[11] = b.n[9];
			fprintf(m_fp,"%8d%5d%8d%8d%8d%8d%8d%8d%8d%8d%8d%8d\n",n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],n[8],n[9],n[10], n[11]);
		}
		else
		{
			fprintf(m_fp,"%8d%5d%8d%8d%8d%8d%8d%8d%8d%8d\n",n[0],n[1],n[2],n[3],n[4],n[5],n[6],n[7],n[8],n[9]);
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportShellElements(NIKE3DProject& prj)
{
	int shells = (int)prj.m_Shell.size();
	if (shells == 0) return true;

	if (m_bcomm)
	{
		fprintf(m_fp,"************************ S H E L L   E L E M E N T   D E C K ************************\n");
	}

	double h[4], zero = 0;
	int n[6];

	for (int i=0; i<shells; ++i)
	{
		NIKE3DProject::SHELL& s = prj.m_Shell[i];
		
		n[0] = i+1;
		n[1] = s.nmat+1;
		n[2] = s.n[0];
		n[3] = s.n[1];
		n[4] = s.n[2];
		n[5] = s.n[3];

		h[0] = s.h[0];
		h[1] = s.h[1];
		h[2] = s.h[2];
		h[3] = s.h[3];

		fprintf(m_fp,"%8d%5d%8d%8d%8d%8d\n",n[0],n[1],n[2],n[3],n[4],n[5]);
		fprintf(m_fp,"%10g%10g%10g%10g%10g\n", h[0], h[1], h[2], h[3], zero);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportDiscrete(NIKE3DProject& prj)
{
	int ndemat = (int)prj.m_DMA.size();
	int numdel = (int)prj.m_DSP.size();
	int nummas = 0;

	// make sure there are any discrete elements
	if (ndemat == 0) return true;

	if (m_bcomm)
	{
		fprintf(m_fp,"**************** D I S C R E T E   E L E M E N T   D E C K ****************\n");
	}

	int i;

	fprintf(m_fp, "%5d%5d%5d\n", ndemat, numdel, nummas);

	for (i=0; i<ndemat; ++i)
	{
		NIKE3DProject::DISCRETE_MATERIAL& m = prj.m_DMA[i];

		fprintf(m_fp, "%5d%5d\n", m.nid, m.ntype);
		fprintf(m_fp, "%10g\n", m.m[0]);
	}

	for (i=0; i<numdel; ++i)
	{
		NIKE3DProject::DISCRETE_SPRING& s = prj.m_DSP[i];
		fprintf(m_fp, "%5d%8d%8d%5d%10g\n", s.nid, s.n1, s.n2, s.nmat, s.s);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportRigid(NIKE3DProject& prj)
{
	int numrnf = (int)prj.m_Rigid.size();
	if (numrnf == 0) return true;

	if (m_bcomm)
	{
		fprintf(m_fp,"**************** R I G I D   N O D E   A N D   F A C E T   D E C K ****************\n");
	}

	list<NIKE3DProject::RIGID_FACET>::iterator pf = prj.m_Rigid.begin();
	for (int i=0; i<numrnf; ++i, ++pf)
	{
		NIKE3DProject::RIGID_FACET& rf = *pf;
		fprintf(m_fp,"%5d", rf.nrb);
		for (int k=0; k<rf.nsize; ++k) fprintf(m_fp, "%8d", rf.node[k]);
		fprintf(m_fp, "\n");
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportSliding(NIKE3DProject& prj)
{
	int nslide = (int)prj.m_SI.size();
	if (nslide == 0) return true;

	if (m_bcomm)
	{
		fprintf(m_fp,"******************* S L I D I N G   I N T E R F A C E   D E C K *******************\n");
	}

	int n[10], i;
	double f[10];

	// first we export all control data
	for (i=0; i<nslide; ++i)
	{
		NIKE3DProject::SLIDING_INTERFACE& si = prj.m_SI[i];

		if (i!=0 && m_bcomm)
		{
			fprintf(m_fp,"*********************************************************************\n");
		}

		n[0] = si.nns;
		n[1] = si.nms;
		n[2] = si.itype;
		f[3] = si.pen;
		f[4] = si.mus;
		f[5] = si.muk;
		f[6] = si.fde;
		f[7] = si.pend;
		n[8] = si.bwrad;
		n[9] = si.aicc;

		if (m_bcomm)
		{
			fprintf(m_fp,"* [ 1] Number of slave facets\n");
			fprintf(m_fp,"* [ 2] Number of master facets\n");
			fprintf(m_fp,"* [ 3] Interface type\n");
			fprintf(m_fp,"* [ 4] Scale factor for sliding surface penalties\n");
			fprintf(m_fp,"* [ 5] Static friction coefficient\n");
			fprintf(m_fp,"* [ 6] Kinetic friction coefficient\n");
			fprintf(m_fp,"* [ 7] Friction decay exponent\n");
			fprintf(m_fp,"* [ 8] Small penetration search distance\n");
			fprintf(m_fp,"* [ 9] Bandwidth minimization radius\n");
			fprintf(m_fp,"* [10] Auxiliary interface control card flag\n");
			fprintf(m_fp,"*--1---|---2---|-3-|----4----|----5----|----6----|----7----|----8----|--9-|-10-|\n");
		}
		fprintf(m_fp, "%8d%8d%4d%10.3g%10.3g%10.3g%10.3g%10.3g%5d%5d\n", n[0], n[1], n[2], f[3], f[4], f[5], f[6], f[7], n[8], n[9]);

		if (n[9] == 1)
		{
			n[0] = si.iaug;
			f[1] = si.toln;
			f[2] = si.tolt;
			f[3] = si.tkmult;
			f[4] = si.tdeath;
			f[5] = si.tbury;
			if (m_bcomm)
			{
				fprintf(m_fp,"*--------------------------------------------------------------------------------\n");
				fprintf(m_fp,"* [ 1] Augmentation flag\n");
				fprintf(m_fp,"* [ 2] Normal direction convergence tolerance for augmentations\n");
				fprintf(m_fp,"* [ 3] Tangential direction convergence tolerance for augmentations\n");
				fprintf(m_fp,"* [ 4] Tangent stiffness multiplier\n");
				fprintf(m_fp,"* [ 5] Interface death time\n");
				fprintf(m_fp,"* [ 6] Interface burial time\n");
				fprintf(m_fp,"*-1-|----2----|----3----|----4----|----5----|----6----|\n");
			}
			fprintf(m_fp, "%5d%10.3g%10.3g%10.3g%10.3g%10.3g\n", n[0], f[1], f[2], f[3], f[4], f[5]);
		}
	}

	// Next, we export all surface data
	list<NIKE3DProject::SI_FACET>::iterator pf = prj.m_Face.begin();
	for (i=0; i<(int) prj.m_Face.size(); ++i, ++pf)
	{
		NIKE3DProject::SI_FACET& f = *pf;
		fprintf(m_fp, "%8d%8d%8d%8d%8d\n", f.nid, f.n[0], f.n[1], f.n[2], f.n[3]);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportLoadCurve(NIKE3DProject& prj)
{
	int nlc = (int)prj.m_LC.size();
	if (nlc == 0) return true;

	if (m_bcomm)
	{
		fprintf(m_fp,"*************************** L O A D   C U R V E   D E C K ***************************\n");
	}

	int i, j;
	list<LoadCurve>::iterator pc = prj.m_LC.begin();
	for (i=0; i<nlc; ++i, ++pc)
	{
		if (i != 0) fprintf(m_fp,"*-----------------------\n");

		LoadCurve& lc = *pc;

		fprintf(m_fp, "%5d%5d\n", i+1, lc.Points());
		for (j=0; j<lc.Points(); ++j)
		{
			vec2d pt = lc.Point(j);
			fprintf(m_fp, "%10lg%10lg\n", pt.x(), pt.y());
		}
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportNodalLoads(NIKE3DProject& prj)
{
	int ncnl = (int)prj.m_NF.size();
	if (ncnl == 0) return true;

	if (m_bcomm)
	{
		fprintf(m_fp,"******************** C O N C E N T R A T E D   N O D A L   L O A D S ********************\n");
	}

	list<NIKE3DProject::NODAL_LOAD>::iterator pn = prj.m_NF.begin();
	for (int i=0; i<ncnl; ++i, ++pn)
	{
		NIKE3DProject::NODAL_LOAD& nl = *pn;
		fprintf(m_fp, "%8d%5d%5d%10lg\n", nl.node, nl.bc, nl.lc, nl.s);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportPressure(NIKE3DProject& prj)
{
	int numpr = (int)prj.m_PF.size();
	if (numpr == 0) return true;

	if (m_bcomm)
	{
		fprintf(m_fp,"********************** P R E S S U R E   B O U N D A R Y   D E C K **********************\n");
	}

	int n[5];
	double f[4];
	list<NIKE3DProject::PRESSURE_LOAD>::iterator pf = prj.m_PF.begin();
	for (int i=0; i<numpr; ++i, ++pf)
	{
		n[0] = pf->lc;
		n[1] = pf->n[0];
		n[2] = pf->n[1];
		n[3] = pf->n[2];
		n[4] = pf->n[3];
		f[0] = pf->s[0];
		f[1] = pf->s[1];
		f[2] = pf->s[2];
		f[3] = pf->s[3];

		fprintf(m_fp, "%5d%8d%8d%8d%8d%10lg%10lg%10lg%10lg\n", n[0], n[1], n[2], n[3], n[4], f[0], f[1], f[2], f[3]);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportDisplacement(NIKE3DProject& prj)
{
	int numdis = (int)prj.m_DC.size();
	if (numdis == 0) return true;

	if (m_bcomm)
	{
		fprintf(m_fp,"******************** P R E S C R I B E D   D I S P L A C E M E N T S ********************\n");
	}

	list<NIKE3DProject::NODAL_DISPLACEMENT>::iterator pn = prj.m_DC.begin();
	for (int i=0; i<numdis; ++i, ++pn)
	{
		NIKE3DProject::NODAL_DISPLACEMENT& dc = *pn;
		fprintf(m_fp, "%8d%5d%5d%10lg%5d\n", dc.node, dc.bc, dc.lc, dc.s, dc.nstat);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportBodyForce(NIKE3DProject& prj)
{
	int nbf = (int)prj.m_BF.size();
	if (nbf == 0) return true;

	if (m_bcomm)
	{
		fprintf(m_fp,"**************************** B O D Y   F O R C E   D E C K ******************************\n");
	}

	list<NIKE3DProject::BODY_FORCE>::iterator pbf = prj.m_BF.begin();
	for (int i=0; i<nbf; ++i, ++pbf)
	{
		NIKE3DProject::BODY_FORCE& bf = *pbf;
		fprintf(m_fp, "%5d%10lg\n", bf.lc, bf.s);
	}

	return true;
}

//-----------------------------------------------------------------------------

bool NIKE3DExport::ExportVelocities(NIKE3DProject& prj)
{
	if (prj.m_Ctrl.intvel == 0) return true;
	int nvel = prj.m_Ctrl.numnp;

	if (m_bcomm)
	{
		fprintf(m_fp,"**************************** I N I T I A L   V E L O C I T I E S ************************\n");
	}

	for (int i=0; i<nvel; ++i)
	{
		NIKE3DProject::NODAL_VELOCITY& v = prj.m_Vel[i];
		fprintf(m_fp, "%8d%10lg%10lg%10lg%5d\n", v.node, v.vx, v.vy, v.vz, v.ninc);
	}

	return true;
}
