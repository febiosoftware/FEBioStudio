#pragma once
#include "FEModelComponent.h"
#include <FSCore/LoadCurve.h>
#include <vector>
#include <string>

class FSModel;
class FSBoundaryCondition;
class FSLoad;
class FSInterface;
class FSRigidConstraint;	// obsolete!
class FSLinearConstraintSet;
class FSRigidLoad;
class FSRigidBC;
class FSRigidIC;
class FSRigidConnector;
class FSInitialCondition;
class FSStepComponent;
class FSModelConstraint;
class FSMeshAdaptor;

//-----------------------------------------------------------------------------
// Analysis types
//
#define FE_STEP_INITIAL				0
#define FE_STEP_MECHANICS			1
#define FE_STEP_NL_DYNAMIC			2	// obsolete (only defined for backward compatibility)
#define FE_STEP_HEAT_TRANSFER		3
#define FE_STEP_BIPHASIC			4
#define FE_STEP_BIPHASIC_SOLUTE	    5
#define FE_STEP_MULTIPHASIC			6
#define FE_STEP_FLUID               7
#define FE_STEP_REACTION_DIFFUSION	8
#define FE_STEP_FLUID_FSI           9
#define FE_STEP_FEBIO_ANALYSIS		10
#define FE_STEP_POLAR_FLUID         11
#define FE_STEP_EXPLICIT_SOLID		12
#define FE_STEP_FLUID_SOLUTES       13
#define FE_STEP_THERMO_FLUID        14

//-----------------------------------------------------------------------------
// This is the base class for step classes
//-----------------------------------------------------------------------------
class FSStep : public FSModelComponent
{
	class Imp;

public:
	FSStep(FSModel* ps, int ntype);
	virtual ~FSStep();

	// get the unique step ID
	int GetID() { return m_nID; }

	// set the ID
	void SetID(int nid);

	// get the step type
	int GetType() { return m_ntype; }

	// I/O
	void Load(IArchive& ar);
	void Save(OArchive& ar);

	// boundary conditions
	int BCs();
	FSBoundaryCondition* BC(int i);
	void AddBC(FSBoundaryCondition* pbc);
	void InsertBC(int n, FSBoundaryCondition* pbc);
	int RemoveBC(FSBoundaryCondition* pbc);
	FSBoundaryCondition* ReplaceBC(int n, FSBoundaryCondition* newBC);
	void RemoveAllBCs();
	int ActiveBCs();

	// loads
	int Loads();
	FSLoad* Load(int i);
	void AddLoad(FSLoad* pfc);
	void InsertLoad(int n, FSLoad* pfc);
	int RemoveLoad(FSLoad* pfc);
	void RemoveAllLoads();
	FSLoad* ReplaceLoad(int n, FSLoad* pl);

	// initial conditions
	int ICs();
	FSInitialCondition* IC(int i);
	void AddIC(FSInitialCondition* pic);
	void InsertIC(int n, FSInitialCondition* pic);
	int RemoveIC(FSInitialCondition* pic);
	void RemoveAllICs();

	// contact interfaces
	int Interfaces();
	FSInterface* Interface(int i);
	void AddInterface(FSInterface* pi);
	void InsertInterface(int n, FSInterface* pi);
	int RemoveInterface(FSInterface* pi);
	void RemoveAllInterfaces();
	void ReplaceInterface(FSInterface* pold, FSInterface* pnew);

	// non-linear constraints
	int Constraints();
	int Constraints(int ntype);
	FSModelConstraint* Constraint(int i);
	void AddConstraint(FSModelConstraint* pc);
	void InsertConstraint(int n, FSModelConstraint* pc);
	void RemoveConstraint(FSModelConstraint* pc);
	void RemoveAllConstraints();

	// rigid constraints
	int RigidConstraints();
	int RigidConstraints(int ntype);
	FSRigidConstraint* RigidConstraint(int i);
	void AddRC(FSRigidConstraint* prc);
	void InsertRC(int n, FSRigidConstraint* prc);
	int RemoveRC(FSRigidConstraint* prc);
	void RemoveAllRigidConstraints();

	// rigid loads
	int RigidLoads();
	int RigidLoads(int ntype);
	FSRigidLoad* RigidLoad(int i);
	void AddRigidLoad(FSRigidLoad* prc);
	void InsertRigidLoad(int n, FSRigidLoad* prc);
	int RemoveRigidLoad(FSRigidLoad* prc);
	void RemoveAllRigidLoads();

	// rigid BC
	int RigidBCs();
	int RigidBCs(int ntype);
	FSRigidBC* RigidBC(int i);
	void AddRigidBC(FSRigidBC* prc);
	void InsertRigidBC(int n, FSRigidBC* prc);
	int RemoveRigidBC(FSRigidBC* prc);
	void RemoveAllRigidBCs();

	// rigid IC
	int RigidICs();
	int RigidICs(int ntype);
	FSRigidIC* RigidIC(int i);
	void AddRigidIC(FSRigidIC* prc);
	void InsertRigidIC(int n, FSRigidIC* prc);
	int RemoveRigidIC(FSRigidIC* prc);
	void RemoveAllRigidICs();

	// linear constraints
	int LinearConstraints();
	FSLinearConstraintSet* LinearConstraint(int i);
	void AddLinearConstraint(FSLinearConstraintSet* plc);
	void RemoveAllLinearConstraints();

    // rigid connectors
	int RigidConnectors();
	FSRigidConnector* RigidConnector(int i);
    void AddRigidConnector(FSRigidConnector* pi);
    void InsertRigidConnector(int n, FSRigidConnector* pi);
    int RemoveRigidConnector(FSRigidConnector* pi);
	void RemoveAllRigidConnectors();

	// mesh adaptors
	int MeshAdaptors();
	FSMeshAdaptor* MeshAdaptor(int i);
	void AddMeshAdaptor(FSMeshAdaptor* pi);
	void InsertMeshAdaptor(int n, FSMeshAdaptor* pi);
	int RemoveMeshAdaptor(FSMeshAdaptor* pi);
	void RemoveAllMeshAdaptors();

	// convenience functions for working with components
	void AddComponent(FSStepComponent* pc);
	void RemoveComponent(FSStepComponent* pc);

	int StepComponents();

public: // ref counting
	static void ResetCounter();
	static void SetCounter(int n);
	static int GetCounter();
	static void DecreaseCounter();

private:
	Imp*		imp;			// implementation class
	int			m_ntype;		// type of step
	int			m_nID;			// ID of step
	static int	m_ncount;		// counter to set the ID in c'tor
};

//-----------------------------------------------------------------------------
// Initial step class. Only one initial step can be defined
//-----------------------------------------------------------------------------
class FSInitialStep : public FSStep
{
public:
	FSInitialStep(FSModel* ps);
	virtual ~FSInitialStep(){}

	// I/O
	void Load(IArchive& ar);
	void Save(OArchive& ar);
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
struct STEP_SETTINGS
{
	// Title/Description
	char	sztitle[256];

	// Time Settings
	int		ntime;			// nr. of time steps
	double	dt;				// time step size
	bool	bauto;			// auto time step controller
	bool	bmust;			// use must points
	int		mxback;			// maximum nr of retries
	int		iteopt;			// optimal nr of retries per step
	double	dtmin;			// minimum step size
	double	dtmax;			// maximum step size
	int		ncut;			// cutback method
	double	tfinal;			// final time
	bool	dtforce;		// force max time step

	// Nonlinear solver Settings
	int		mthsol;			// nonlinear equilibrium solution method
	int		ilimit;			// max nr of eq iters permitted between SMR's
	int		maxref;			// max nr of SMR's per time step
	bool	bdivref;		// reform on divergence
	bool	brefstep;		// reform each time step
	bool	logSolve;		// log solver flag

    // linear solver settings
	bool	bminbw;			// minimze bandwidth
	int		nmatfmt;		// matrix storage format: 0 = default, 1 = symmetric, 2 = non-symmetric
	int		neqscheme;		// equation scheme, 0 = default, 1 = block

	// analysis settings
	int		nanalysis;		// type of analysis

	// time integration settings for time-dependent problems
	bool	override_rhoi;	// flag indicating whether the spectral radius (defined in the nonlinear solver classes should be overridden)
    double  alpha;          // alpha parameter Hilber-Hughes-Taylor method
    double  beta;           // beta parameter Hilber-Hughes-Taylor method
    double  gamma;          // gamma parameter Hilber-Hughes-Taylor method

	// output options
	int	plot_level;		// plot level
	int	plot_stride;	// plot stride parameter
	bool plot_zero;		// plot zero state
	int	plot_range[2];	// plot range
	int output_level;	// output level

	bool adapter_re_solve;

	// solute constants
//	double	Rc;				// universal gas constant
//	double	Ta;				// absolute temperature

	// set default values
	void Defaults();
};

//-----------------------------------------------------------------------------
// This class defines the base class for all analysis steps
//-----------------------------------------------------------------------------
class FSAnalysisStep : public FSStep
{
public:
	// destructor
	virtual ~FSAnalysisStep(){}

	// I/O
	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// get the step-settings
	STEP_SETTINGS& GetSettings() { return m_ops; }

	// get the must-point load curve
	LoadCurve* GetMustPointLoadCurve() { return &m_MP; }

	// get the analysis types
	virtual std::vector<std::string> GetAnalysisStrings() const;

protected:
	// constructor is private since we don't want to create instances of the base class
	FSAnalysisStep(FSModel* ps, int ntype);

protected:
	STEP_SETTINGS	m_ops;		// step options
	LoadCurve		m_MP;		// must-point curve
};

//-----------------------------------------------------------------------------
class FSNonLinearMechanics : public FSAnalysisStep
{
	enum { MP_DTOL, MP_ETOL, MP_RTOL, MP_LSTOL, MP_MINRES, MP_QNMETHOD };

public:
	FSNonLinearMechanics(FSModel* ps);

	double GetDisplacementTolerance();
	double GetEnergyTolerance();
	double GetResidualTolerance();
	double GetLineSearchTolerance();

	void SetDisplacementTolerance(double dtol);
	void SetEnergyTolerance(double etol);
	void SetResidualTolerance(double rtol);
	void SetLineSearchTolerance(double lstol);
};

//-----------------------------------------------------------------------------
class FSExplicitSolidAnalysis : public FSAnalysisStep
{
public:
	FSExplicitSolidAnalysis(FSModel* fem);
};

//-----------------------------------------------------------------------------
class FSHeatTransfer : public FSAnalysisStep
{
public:
	FSHeatTransfer(FSModel* ps);

	// get the analysis types
	std::vector<std::string> GetAnalysisStrings() const;
};

//-----------------------------------------------------------------------------
class FSNonLinearBiphasic : public FSAnalysisStep
{
public:
	FSNonLinearBiphasic(FSModel* ps);

	// get the analysis types
	std::vector<std::string> GetAnalysisStrings() const;
};

//-----------------------------------------------------------------------------
class FSBiphasicSolutes : public FSAnalysisStep
{
public:
	FSBiphasicSolutes(FSModel* ps);

	// get the analysis types
	std::vector<std::string> GetAnalysisStrings() const;
};

//-----------------------------------------------------------------------------
class FSMultiphasicAnalysis : public FSAnalysisStep
{
public:
	FSMultiphasicAnalysis(FSModel* ps);

	// get the analysis types
	std::vector<std::string> GetAnalysisStrings() const;
};

//-----------------------------------------------------------------------------
class FSFluidFSIAnalysis : public FSAnalysisStep
{
public:
	FSFluidFSIAnalysis(FSModel* ps);
};

//-----------------------------------------------------------------------------
class FSFluidAnalysis : public FSAnalysisStep
{
public:
    FSFluidAnalysis(FSModel* ps);
};

//-----------------------------------------------------------------------------
class FSReactionDiffusionAnalysis : public FSAnalysisStep
{
public:
	FSReactionDiffusionAnalysis(FSModel* ps);

	// get the analysis types
	std::vector<std::string> GetAnalysisStrings() const;
};

//-----------------------------------------------------------------------------
class FSPolarFluidAnalysis : public FSAnalysisStep
{
public:
    FSPolarFluidAnalysis(FSModel* ps);
};

//-----------------------------------------------------------------------------
class FSFluidSolutesAnalysis : public FSAnalysisStep
{
public:
    FSFluidSolutesAnalysis(FSModel* ps);
};

//-----------------------------------------------------------------------------
class FSThermoFluidAnalysis : public FSAnalysisStep
{
public:
    FSThermoFluidAnalysis(FSModel* ps);
};

//==============================================================================
class FEBioAnalysisStep : public FSStep
{
public:
	FEBioAnalysisStep(FSModel* ps);
	void Save(OArchive& ar);
	void Load(IArchive& ar);
};
