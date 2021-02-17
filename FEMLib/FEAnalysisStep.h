#pragma once
#include <FSCore/FSObject.h>
#include <vector>
//using namespace std;

class FEModel;
class FEBoundaryCondition;
class FELoad;
class FEInterface;
class FERigidConstraint;
class FELinearConstraintSet;
class FERigidConnector;
class FEInitialCondition;
class FEStepComponent;
class FEModelConstraint;

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

//-----------------------------------------------------------------------------
// This is the base class for step classes
//-----------------------------------------------------------------------------
class FEStep : public FSObject
{
	class Imp;

public:
	FEStep(FEModel* ps, int ntype);
	virtual ~FEStep();

	// get the unique step ID
	int GetID() { return m_nID; }

	// set the ID
	void SetID(int nid);

	// get the step type
	int GetType() { return m_ntype; }

	// get/set type string
	void SetTypeString(const char* sztype) { m_sztype = sztype; }
	const char* GetTypeString() { return m_sztype; }

	//! get the model
	//! \todo I don't think this is being used)
	FEModel* GetFEModel() { return m_pfem; }

	// I/O
	void Load(IArchive& ar);
	void Save(OArchive& ar);

	// boundary conditions
	int BCs();
	FEBoundaryCondition* BC(int i);
	void AddBC(FEBoundaryCondition* pbc);
	void InsertBC(int n, FEBoundaryCondition* pbc);
	int RemoveBC(FEBoundaryCondition* pbc);
	void RemoveAllBCs();
	int ActiveBCs();

	// loads
	int Loads();
	FELoad* Load(int i);
	void AddLoad(FELoad* pfc);
	void InsertLoad(int n, FELoad* pfc);
	int RemoveLoad(FELoad* pfc);
	void RemoveAllLoads();

	// initial conditions
	int ICs();
	FEInitialCondition* IC(int i);
	void AddIC(FEInitialCondition* pic);
	void InsertIC(int n, FEInitialCondition* pic);
	int RemoveIC(FEInitialCondition* pic);
	void RemoveAllICs();

	// contact interfaces
	int Interfaces();
	FEInterface* Interface(int i);
	void AddInterface(FEInterface* pi);
	void InsertInterface(int n, FEInterface* pi);
	int RemoveInterface(FEInterface* pi);
	void RemoveAllInterfaces();

	// non-linear constraints
	int Constraints();
	int Constraints(int ntype);
	FEModelConstraint* Constraint(int i);
	void AddConstraint(FEModelConstraint* pc);
	void InsertConstraint(int n, FEModelConstraint* pc);
	void RemoveConstraint(FEModelConstraint* pc);
	void RemoveAllConstraints();

	// rigid constraints
	int RigidConstraints();
	int RigidConstraints(int ntype);
	FERigidConstraint* RigidConstraint(int i);
	void AddRC(FERigidConstraint* prc);
	void InsertRC(int n, FERigidConstraint* prc);
	int RemoveRC(FERigidConstraint* prc);
	void RemoveAllRigidConstraints();

	// linear constraints
	int LinearConstraints();
	FELinearConstraintSet* LinearConstraint(int i);
	void AddLinearConstraint(FELinearConstraintSet* plc);
	void RemoveAllLinearConstraints();

    // rigid connectors
	int RigidConnectors();
	FERigidConnector* RigidConnector(int i);
    void AddRigidConnector(FERigidConnector* pi);
    void InsertRigidConnector(int n, FERigidConnector* pi);
    int RemoveRigidConnector(FERigidConnector* pi);
	void RemoveAllRigidConnectors();

	// convenience functions for working with components
	void AddComponent(FEStepComponent* pc);
	void RemoveComponent(FEStepComponent* pc);
    

public: // ref counting
	static void ResetCounter();
	static void SetCounter(int n);
	static int GetCounter();
	static void DecreaseCounter();

protected:
	FEModel*	m_pfem;	// pointer to FEModel class

private:
	Imp*		imp;			// implementation class
	int			m_ntype;		// type of step
	const char*	m_sztype;		// type string (set by c'tor of derived classes)
	int			m_nID;			// ID of step
	static int	m_ncount;		// counter to set the ID in c'tor
};

//-----------------------------------------------------------------------------
// Initial step class. Only one initial step can be defined
//-----------------------------------------------------------------------------
class FEInitialStep : public FEStep
{
public:
	FEInitialStep(FEModel* ps);
	virtual ~FEInitialStep(){}

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

	// Nonlinear solver Settings
	int		mthsol;			// nonlinear equilibrium solution method
	int		ilimit;			// max nr of eq iters permitted between SMR's
	int		maxref;			// max nr of SMR's per time step
	bool	bdivref;		// reform on divergence
	bool	brefstep;		// reform each time step

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

	// solute constants
//	double	Rc;				// universal gas constant
//	double	Ta;				// absolute temperature

	// set default values
	void Defaults();
};

//-----------------------------------------------------------------------------
// This class defines the base class for all analysis steps
//-----------------------------------------------------------------------------
class FEAnalysisStep : public FEStep
{
public:
	// destructor
	virtual ~FEAnalysisStep(){}

	// I/O
	void Save(OArchive& ar);
	void Load(IArchive& ar);

	// get the step-settings
	STEP_SETTINGS& GetSettings() { return m_ops; }

	// get the must-point load curve
	FELoadCurve* GetMustPointLoadCurve() { return &m_MP; }

	// get the analysis types
	virtual vector<string> GetAnalysisStrings() const;

protected:
	// constructor is private since we don't want to create instances of the base class
	FEAnalysisStep(FEModel* ps, int ntype);

protected:
	STEP_SETTINGS	m_ops;		// step options
	FELoadCurve		m_MP;		// must-point curve
};

//-----------------------------------------------------------------------------
class FENonLinearMechanics : public FEAnalysisStep
{
	enum { MP_DTOL, MP_ETOL, MP_RTOL, MP_LSTOL, MP_MINRES, MP_QNMETHOD };

public:
	FENonLinearMechanics(FEModel* ps);

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
class FEHeatTransfer : public FEAnalysisStep
{
public:
	FEHeatTransfer(FEModel* ps);

	// get the analysis types
	vector<string> GetAnalysisStrings() const;
};

//-----------------------------------------------------------------------------
class FENonLinearBiphasic : public FEAnalysisStep
{
public:
	FENonLinearBiphasic(FEModel* ps);

	// get the analysis types
	vector<string> GetAnalysisStrings() const;
};

//-----------------------------------------------------------------------------
class FEBiphasicSolutes : public FEAnalysisStep
{
public:
	FEBiphasicSolutes(FEModel* ps);

	// get the analysis types
	vector<string> GetAnalysisStrings() const;
};

//-----------------------------------------------------------------------------
class FEMultiphasicAnalysis : public FEAnalysisStep
{
public:
	FEMultiphasicAnalysis(FEModel* ps);

	// get the analysis types
	vector<string> GetAnalysisStrings() const;
};

//-----------------------------------------------------------------------------
class FEFluidFSIAnalysis : public FEAnalysisStep
{
public:
	FEFluidFSIAnalysis(FEModel* ps);
};

//-----------------------------------------------------------------------------
class FEFluidAnalysis : public FEAnalysisStep
{
public:
    FEFluidAnalysis(FEModel* ps);
};

//-----------------------------------------------------------------------------
class FEReactionDiffusionAnalysis : public FEAnalysisStep
{
public:
	FEReactionDiffusionAnalysis(FEModel* ps);

	// get the analysis types
	vector<string> GetAnalysisStrings() const;
};
