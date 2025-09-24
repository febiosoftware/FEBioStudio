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

#pragma once

#include <FEMLib/FSProject.h>
#include <vector>
#include <list>

class NIKE3DProject
{
public:
	struct CONTROL
	{
		// CC1
		char	sztitle[72];	// title

		// CC2
		int		nmmat;		// nr of materials
		int		numnp;		// nr of node points
		int		numelh;		// nr of brick elements
		int		numelb;		// nr of beam elements
		int		numels;		// nr of shell elements
		int		num1d;		// nr of 1D slide lines
		int		numsi;		// nr of sliding interfaces
		int		nrwsp;		// nr of rigid walls
		int		inpde;		// discreet element input flag
		int		numrnf;		// nr of rigid nodes and facets

		// CC3
		int		ntime;		// nr of time steps
		double	dt;			// time step size
		int		nauto;		// auto-time stepper
		int		mxback;		// max nr of retries allowable per step
		int		iteopt;		// optimal nr of iterations
		double	dtmin;		// min time step size
		double	dtmax;		// max time step size
		int		irfwin;		// size of opt iteration window
		int		spf;		// super plastic forming flag
		int		linearb;	// brick analysis flag
		int		linears;	// shell analysis flag
		int		linearbm;	// beam analysis flag
		int		linearpr;	// pressure analysis flag

		// CC4
		int		numlc;		// nr of load curves
		int		nptm;		// max nr of points on any lc
		int		numcnl;		// nr of concentrated nodal loads
		int		numpr;		// nr of element surfaces having pressure loads
		int		numdis;		// nr of displacement BC cards
		int		numadl;		// nr of beam elements with aero-drag
		int		nrcc;		// nr of node constraint cards
		int		bfa[3];		// body force
		int		bfav[3];	// body force ang vel in x
		int		nsteer;		// nr of nodes with steering BC
		int		nfnbc;		// nr of nodes with foundation BC

		// CC5
		int		ipri;		// output printing interval
		int		jpri;		// output plotting interval
		int		nnpb;		// nr of node print blocks
		int		nhpb;		// nr of brick print blocks
		int		nbpb;		// nr of beam print blocks
		int		nspb;		// nr of shell print blocks
		int		jrfreq;		// nr of time steps between running restart file generation
		int		irfreq;		// nr of time steps between restart file generation
		int		istrn;		// shell element strain dump flag
		int		sw[5];		// switch toggle flags
		int		iacflg;		// auxilliary data dump flag
		int		maxaug;		// max aug lag iters per time step
		int		ingap;		// interface gap plot flag

		// CC6
		int		mthsol;		// nonlinear solution method
		int		sflag;		// solver specific flag
		int		icnt1;		// nr of steps between stiff reform at beginning of step
		int		icnt2;		// nr of time steps between equil iterations
		int		ilimit;		// max nr of quasi-Newton iters
		int		maxref;		// max nr of reforms
		double	dtol;		// displacement tolerance
		double	ectl;		// energy tolerance
		double	rctl;		// residual tolerance
		double	tolls;		// line search tolerance
		double	inref;		// nr of initial stiff formations

		// CC7
		int		imass;		// analysis type
		int		intvel;		// initial condition flag
		int		iteo;		// thermal effects option
		int		itpro;		// temperature profile flag
		int		neig;		// nr of eigenvalues
		double	eshift;		// frequency shift
		double	fnip;		// first newark integration param
		double	snip;		// second newark integration param
		double	alpha;		// HHT integration param
		double	etol;		// eigen solver convergence tol
		int		ieigit;		// eigen solver max iters
		int		ieigens;	// eigensolver solution method

		// CC8
		int		iunsym;		// symmetric/unsymmetric stiffness flag
		int		nwebin;		// element data buffer size
		int		ifiss;		// direct linear solver
		int		ioroin;		// BFGS update vector storage
		int		ibrfor;		// brick formulation flag
		int		igsoin;		// brick geomety stiffness
		int		ishfin;		// shell formulation flag
		double	qhg;		// hourglass control parameter
		int		isgsin;		// shell geometry stiffness
		int		ibmfin;		// beam formulation flag
		int		ibgsin;		// beam geometry stiffness
		int		istold;		// contact search and linearization flag
		int		irotary;	// rotary inertia flag
		int		geotol;		// geometric stiffness init threshold

		// CC9
		double	dsx;		// desired initial arc length
		int		irco;		// arc length constraint method
		int		nusbir;		// nr of user integration rules for beams
		int		mpubr;		// max nr of beam integration points
		int		nussir;		// nr of user integration rules for shells
		int		mpusr;		// max nr of shell integration points

		// CC10
		int		itrsol;		// iterative linear solver
		int		itrlmt;		// iteratio nlimit
		double	tollin;		// iteration convergence tolerance
		int		lbuf;		// buffer size
		int		itrpnt;		// print-out option for linear iterative solver
		int		iebeoc;		// iterative solver data storage option
	};

	struct MATERIAL
	{
		int		ntype;		// material type
		double	dens;		// material density
		int		nelem;		// element class
		double	Tref;		// material reference temperature
		double	rda;		// Rayleigh damping alpha
		double	rdb;		// Rayleigh damping beta
		double	hrgl;		// hourglass penalty scale
		double	flag;		// formulation flags

		double	m[8][8];	// material parameters
		char	szname[72];
	};

	struct NODE
	{
		double	x, y, z;	// node position
		int		rc, bc;		// nodal constraints
	};

	struct BRICK
	{
		int		n[20];		// nodal connectivity
		int		nmat;		// material ID
		int		nnew;		// flag indicating whether this requires the "newelem" keyword (for some higher order elements)
	};

	struct SHELL
	{
		int		n[4];		// nodal connectivity
		int		nmat;		// material ID
		double	h[4];		// shell thickness
	};

	struct RIGID_FACET
	{
		int		nrb;		// rigid body material ID
		int		nsize;		// nr of nodes
		int		node[4];	// node numbers
	};

	struct NODAL_LOAD
	{
		int		node;	// node point number
		int		bc;		// direction of force
		int		lc;		// load curve number
		double	s;		// scale factor

	};

	struct PRESSURE_LOAD
	{
		int	lc;			// load curve number
		int	n[4];		// node numbers
		double s[4];	// scale factors
	};

	struct NODAL_DISPLACEMENT
	{
		int	node;	// node
		int bc;		// direction of displacement
		int	lc;		// load curve number
		double	s;	// scale factor
		int	nstat;	// static initialization flag
	};

	struct NODAL_VELOCITY
	{
		int		node;		// node number
		double	vx, vy, vz;	// nodal velocities
		int		ninc;		// nodal increment
	};

	struct SLIDING_INTERFACE
	{
		// CC1
		int		nns;	// number of slaves facets
		int		nms;	// number of master facets
		int		itype;	// interface type
		double	pen;	// penalty
		double	mus;	// static friction
		double	muk;	// kinetic friction
		double	fde;	// friction delay exponent
		double	pend;	// small penetration search distance
		int		bwrad;	// bandwidth min radius
		int		aicc;	// auxiliary intf card

		// Aux CC
		int		iaug;		// augmentation flag
		double	toln;		// normal convergence tolerance
		double	tolt;		// tangential convergence tolerance
		double	tkmult;		// tangent stiffness multiplier
		double	tdeath;		// interface death time
		double	tbury;		// interface burial time
	};

	struct SI_FACET
	{
		int	nid;	// facet number
		int n[4];	// facet nodes
	};

	struct BODY_FORCE
	{
		int		lc;	// load curve number
		double	s;	// scale factor
	};

	struct DISCRETE_SPRING
	{
		int		nid;	// element id
		int		n1, n2;	// node numbers
		int		nmat;	// material id
		double	s;		// scale factor
	};

	struct DISCRETE_MATERIAL
	{
		int nid;		// material id
		int	ntype;		// material type
		double	m[5];	// material parameters
	};

public:
	NIKE3DProject() { Defaults(); }

	bool Create(FSProject& prj);

	// builds an FSProject from a nike project
	bool Convert(FSProject& prj);

protected:
	bool BuildControl        (FSProject& prj);
	bool BuildMaterials      (FSProject& prj);
	bool BuildNodes          (FSProject& prj);
	bool BuildElements       (FSProject& prj);
	bool BuildRigidNodes     (FSProject& prj);
	bool BuildDiscrete		 (FSProject& prj);
	bool BuildInterfaces     (FSProject& prj);
	bool BuildNodalLoads     (FSProject& prj);
	bool BuildPressureLoads  (FSProject& prj);
	bool BuildDisplacements  (FSProject& prj);
	bool BuildBodyForce      (FSProject& prj);
	bool BuildNodalVelocities(FSProject& prj);

	int AddLoadCurve(LoadCurve& lc);

protected:
	bool ConvertMaterials(FSProject& prj);
	void Defaults();

public:
	CONTROL							m_Ctrl;
	std::vector<MATERIAL>			m_Mat;
	std::vector<NODE>				m_Node;
	std::vector<BRICK>				m_Brick;
	std::vector<SHELL>				m_Shell;
	std::list<RIGID_FACET>			m_Rigid;
	std::vector<SLIDING_INTERFACE>	m_SI;
	std::list<SI_FACET>				m_Face;
	std::list<LoadCurve>			m_LC;
	std::list<NODAL_LOAD>			m_NF;
	std::list<PRESSURE_LOAD>		m_PF;
	std::list<NODAL_DISPLACEMENT>	m_DC;
	std::list<BODY_FORCE>			m_BF;
	std::vector<NODAL_VELOCITY>		m_Vel;
	std::vector<DISCRETE_SPRING>	m_DSP;
	std::vector<DISCRETE_MATERIAL>	m_DMA;
};
