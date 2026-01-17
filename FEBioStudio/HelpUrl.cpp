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

#include <unordered_map>
#include <FECore/fecore_enum.h>
#include "WebDefines.h"
#include "HelpUrl.h"
#include <stdexcept>

using std::string;
using std::unordered_map;

const unordered_map<int, unordered_map<string, string>> URLs =
{
    // Materials
    {
        FEMATERIAL_ID,
        {
            { "2D fiber neo-Hookean", "" },
            { "Carreau-Yasuda viscous solid", CARREAU_YASUDA_MODEL_HTML },
            { "cell growth", CELL_GROWTH_HTML },
            { "cubic CLE", CUBIC_CLE_HTML },
            { "damage neo-Hookean", "" },
            { "Donnan equilibrium", DONNAN_EQUILIBRIUM_SWELLING_HTML },
            { "EFD Donnan equilibrium", ELLIPSOIDAL_FIBER_DISTRIBUTION_WITH_DONNAN_EQUILIBRIUM_SWELLING_HTML },
            { "EFD neo-Hookean (new)", ELLIPSOIDAL_FIBER_DISTRIBUTION_NEO_HOOKEAN_HTML },
            { "EFD neo-Hookean", ELLIPSOIDAL_FIBER_DISTRIBUTION_NEO_HOOKEAN_HTML },
            { "ellipsoidal fiber distribution", ELLIPSOIDAL_FIBER_DISTRIBUTION_HTML },
            { "ellipsoidal fiber distribution (old)", ELLIPSOIDAL_FIBER_DISTRIBUTION_HTML },
            { "Fung-ortho-compressible", FUNG_ORTHOTROPIC_COMPRESSIBLE_HTML },
            { "compressible Gent", "" },
            { "Holmes-Mow", HOLMES_MOW_HTML },
            { "HGO unconstrained", HOLZAPFEL_GASSER_OGDEN_UNCONSTRAINED_HTML },
            { "isotropic elastic", ISOTROPIC_ELASTIC_HTML },
            { "coupled Mooney-Rivlin", COUPLED_MOONEY_RIVLIN_HTML },
            { "coupled Veronda-Westmann", COUPLED_VERONDA_WESTMANN_HTML },
            { "natural neo-Hookean", NATURAL_NEO_HOOKEAN_HTML },
            { "neo-Hookean", NEO_HOOKEAN_HTML },
            { "neo-Hookean transiso", "" },
            { "Newtonian viscous solid", NEWTONIAN_FLUID_HTML },
            { "Ogden unconstrained", OGDEN_UNCONSTRAINED_HTML },
            { "orthotropic elastic", ORTHOTROPIC_ELASTIC_HTML },
            { "orthotropic CLE", ORTHOTROPIC_CLE_HTML },
            { "osmotic virial expansion", OSMOTIC_PRESSURE_FROM_VIRIAL_EXPANSION_HTML },
            { "perfect osmometer", PERFECT_OSMOMETER_EQUILIBRIUM_OSMOTIC_PRESSURE_HTML },
            { "spherical fiber distribution", SPHERICAL_FIBER_DISTRIBUTION_HTML },
            { "St.Venant-Kirchhoff", "" },
            { "viscoelastic", VISCOELASTIC_SOLIDS_HTML },
            { "multigeneration", MULTIGENERATION_SOLIDS_HTML },
            { "remodeling solid", "" },
            { "Carter-Hayes (old)", CARTER_HAYES_HTML },
            { "continuous fiber distribution", CONTINUOUS_FIBER_DISTRIBUTION_HTML },
            { "coupled trans-iso Veronda-Westmann", COUPLED_TRANSVERSELY_ISOTROPIC_VERONDA_WESTMANN_HTML },
            { "coupled trans-iso Mooney-Rivlin", COUPLED_TRANSVERSELY_ISOTROPIC_MOONEY_RIVLIN_HTML },
            { "hyperelastic", "" },
            { "trans-iso hyperelastic", "" },
            { "damage fiber power", DAMAGE_FIBER_POWER_HTML },
            { "damage fiber exponential", DAMAGE_FIBER_EXPONENTIAL_HTML },
            { "damage fiber exp-linear", DAMAGE_FIBER_EXP_LINEAR_HTML },
            { "solid mixture", SOLID_MIXTURE_HTML },
            { "reactive viscoelastic", REACTIVE_VISCOELASTIC_SOLID_HTML },
            { "elastic damage", "" },
            { "reactive viscoelastic damage", "" },
            { "reactive fatigue", "" },
            { "reactive plasticity", REACTIVE_PLASTICITY_HTML },
            { "reactive plastic damage", REACTIVE_ELASTO_PLASTIC_DAMAGE_MECHANICS_HTML },
            { "Arruda-Boyce", ARRUDA_BOYCE_HTML },
            { "2D trans iso Mooney-Rivlin", TRANSVERSELY_ISOTROPIC_MOONEY_RIVLIN_HTML },
            { "2D trans iso Veronda-Westmann", TRANSVERSELY_ISOTROPIC_VERONDA_WESTMANN_HTML },
            { "damage Mooney-Rivlin", "" },
            { "damage trans iso Mooney-Rivlin", "" },
            { "EFD Mooney-Rivlin", ELLIPSOIDAL_FIBER_DISTRIBUTION_MOONEY_RIVLIN_HTML },
            { "EFD uncoupled", ELLIPSOIDAL_FIBER_DISTRIBUTION_UNCOUPLED_HTML },
            { "EFD Veronda-Westmann", ELLIPSOIDAL_FIBER_DISTRIBUTION_VERONDA_WESTMANN_HTML },
            { "Fung orthotropic", FUNG_ORTHOTROPIC_HTML },
            { "Holzapfel-Gasser-Ogden", HOLZAPFEL_GASSER_OGDEN_HTML },
            { "Gent", "" },
            { "incomp neo-Hookean", "" },
            { "Mooney-Rivlin", MOONEY_RIVLIN_HTML },
            { "muscle material", MUSCLE_MATERIAL_HTML },
            { "Newtonian viscous solid uncoupled", "" },
            { "Ogden", OGDEN_HTML },
            { "TC nonlinear orthotropic", TENSION_COMPRESSION_NONLINEAR_ORTHOTROPIC_HTML },
            { "tendon material", TENDON_MATERIAL_HTML },
            { "trans iso Mooney-Rivlin", TRANSVERSELY_ISOTROPIC_MOONEY_RIVLIN_HTML },
            { "trans iso Veronda-Westmann", TRANSVERSELY_ISOTROPIC_VERONDA_WESTMANN_HTML },
            { "uncoupled solid mixture", UNCOUPLED_SOLID_MIXTURE_HTML },
            { "Veronda-Westmann", VERONDA_WESTMANN_HTML },
            { "uncoupled viscoelastic", UNCOUPLED_VISCOELASTIC_MATERIALS_HTML },
            { "Mooney-Rivlin von Mises Fibers", MOONEY_RIVLIN_VON_MISES_DISTRIBUTED_FIBERS_HTML },
            { "uncoupled active contraction", CONTRACTION_IN_MIXTURES_OF_UNCOUPLED_MATERIALS_HTML },
            { "continuous fiber distribution uncoupled", UNCOUPLED_CONTINUOUS_FIBER_DISTRIBUTION_HTML },
            { "PRLig", "" },
            { "uncoupled reactive viscoelastic", REACTIVE_VISCOELASTIC_SOLID_HTML },
            { "uncoupled elastic damage", "" },
            { "uncoupled hyperelastic", "" },
            { "uncoupled trans-iso hyperelastic", "" },
            { "Kamensky", "" },
            { "Kamensky uncoupled", "" },
            { "fiber-exp-pow", FIBER_WITH_EXPONENTIAL_POWER_LAW_HTML },
            { "fiber-exp-pow-uncoupled", FIBER_WITH_EXPONENTIAL_POWER_LAW_UNCOUPLED_FORMULATION_HTML },
            { "fiber neo-Hookean", FIBER_WITH_NEO_HOOKEAN_LAW_HTML },
            { "fiber-pow-linear", FIBER_WITH_TOE_LINEAR_RESPONSE_HTML },
            { "fiber-pow-linear-uncoupled", FIBER_WITH_TOE_LINEAR_RESPONSE_UNCOUPLED_FORMULATION_HTML },
            { "fiber-exponential-power-law-uncoupled", FIBER_WITH_EXPONENTIAL_POWER_LAW_UNCOUPLED_FORMULATION_HTML },
            { "fiber-NH", FIBER_WITH_NEO_HOOKEAN_LAW_HTML },
            { "fiber-natural-NH", FIBER_WITH_NATURAL_NEO_HOOKEAN_LAW_HTML },
            { "fiber-NH-uncoupled", "" },
            { "fiber-power-linear", "" },
            { "fiber-exp-linear", "" },
            { "uncoupled fiber-exp-linear", "" },
            { "fiber-Kiousis-uncoupled", FIBER_KIOUSIS_UNCOUPLED_HTML },
            { "fiber-exponential-power-law", FIBER_WITH_EXPONENTIAL_POWER_LAW_HTML },
            { "rigid body", RIGID_BODY_HTML },
            { "von-Mises plasticity", "" },
            { "linear truss", "" },
            { "active_contraction", ACTIVE_CONTRACTION_HTML },
            { "wrinkle Ogden", "" },
            { "elastic membrane", "" },
            { "prescribed uniaxial active contraction", PRESCRIBED_UNIAXIAL_ACTIVE_CONTRACTION_HTML },
            { "uncoupled prescribed uniaxial active contraction", UNCOUPLED_PRESCRIBED_UNIAXIAL_ACTIVE_CONTRACTION_HTML },
            { "prescribed trans iso active contraction", PRESCRIBED_TRANSVERSELY_ISOTROPIC_ACTIVE_CONTRACTION_HTML },
            { "uncoupled prescribed trans iso active contraction", UNCOUPLED_PRESCRIBED_TRANSVERSELY_ISOTROPIC_ACTIVE_CONTRACTION_HTML },
            { "prescribed isotropic active contraction", PRESCRIBED_ISOTROPIC_ACTIVE_CONTRACTION_HTML },
            { "uncoupled prescribed isotropic active contraction", UNCOUPLED_PRESCRIBED_ISOTROPIC_ACTIVE_CONTRACTION_HTML },
            { "active fiber stress", "" },
            { "uncoupled active fiber stress", "" },
            { "PFC paper", "" },
            { "PFC user", "" },
            { "PFC math", "" },
            { "prestrain elastic", PRESTRAIN_MATERIAL_HTML },
            { "uncoupled prestrain elastic", PRESTRAIN_MATERIAL_HTML },
            { "Carter-Hayes", CARTER_HAYES_HTML },
            { "porous neo-Hookean", POROUS_NEO_HOOKEAN_HTML },
            { "biphasic", BIPHASIC_MATERIALS_HTML },
            { "biphasic-solute", BIPHASIC_SOLUTE_MATERIALS_HTML },
            { "triphasic", TRIPHASIC_AND_MULTIPHASIC_MATERIALS_HTML },
            { "multiphasic", TRIPHASIC_AND_MULTIPHASIC_MATERIALS_HTML },
            { "multiphasic-multigeneration", MULTIGENERATION_SOLIDS_HTML },
            { "spherical fiber distribution sbm", SPHERICAL_FIBER_DISTRIBUTION_FROM_SOLID_BOUND_MOLECULE_HTML },
            { "fiber-exp-pow sbm", "" },
            { "fiber-pow-linear sbm", "" },
            { "fluid", VISCOUS_FLUID_MATERIALS_HTML },
            { "ideal gas isentropic", "" },
            { "ideal gas isothermal", "" },
            { "fluid-FSI", GENERAL_SPECIFICATION_OF_FLUID_FSI_MATERIALS_HTML },
            { "biphasic-FSI", GENERAL_SPECIFICATION_OF_BIPHASIC_FSI_MATERIALS_HTML },
            { "multiphasic-FSI", "" },
            { "fluid-solutes", "" },
            { "solutes", SOLUTES_HTML },
            { "fluid-solutes2", "" },
            { "thermo-fluid", "" },
            { "micro-material", "" },
            { "micro-material2O", "" },
            { "mindlin elastic", "" }
        }
    },
    // Boundary Conditions
    {
        FEBC_ID,
        {
            { "fix", ZERO_DISPLACEMENT_HTML },
            { "prescribe", PRESCRIBED_DISPLACEMENT_HTML },
            { "linear constraint", LINEAR_CONSTRAINTS_HTML },
            { "zero displacement", ZERO_DISPLACEMENT_HTML },
            { "zero rotation", ZERO_DISPLACEMENT_HTML },
            { "prescribed displacement", PRESCRIBED_DISPLACEMENT_HTML },
            { "prescribed rotation", PRESCRIBED_DISPLACEMENT_HTML },
            { "prescribed deformation", PRESCRIBED_DISPLACEMENT_HTML },
            { "normal displacement", "" },
            { "rigid deformation", "" },
            { "prescribed deformation 2O", "" },
            { "rigid", RIGID__HTML },
            { "zero fluid pressure", ZERO_DISPLACEMENT_HTML },
            { "prescribed fluid pressure", PRESCRIBED_DISPLACEMENT_HTML },
            { "zero concentration", ZERO_DISPLACEMENT_HTML },
            { "prescribed concentration", PRESCRIBED_DISPLACEMENT_HTML },
            { "zero fluid velocity", ZERO_DISPLACEMENT_HTML },
            { "prescribed fluid velocity", PRESCRIBED_DISPLACEMENT_HTML },
            { "zero fluid dilatation", ZERO_DISPLACEMENT_HTML },
            { "prescribed fluid dilatation", PRESCRIBED_DISPLACEMENT_HTML },
            { "fluid rotational velocity", ZERO_DISPLACEMENT_HTML }
        }
    },
    // Loads
    {
        FELOAD_ID,
        {
            { "nodal_load", NODAL_LOAD_HTML },
            { "nodal_force", NODAL_FORCE_HTML },
            { "pressure", PRESSURE_LOAD_HTML },
            { "traction", TRACTION_LOAD_HTML },
            { "const", "" },
            { "non-const", "" },
            { "body force", CONSTANT_BODY_FORCE_HTML },
            { "centrifugal", CENTRIFUGAL_BODY_FORCE_HTML },
            { "point", "" },
            { "surface attraction", SURFACE_ATTRACTION_HTML },
            { "mass damping", "" },
            { "rigid_axial_force", "" },
            { "rigid_force", "" },
            { "rigid_follower_force", "" },
            { "rigid_follower_moment", "" },
            { "rigid_cable", RIGID_CABLE_HTML },
            { "normal_traction", "" },
            { "fluidflux", FLUID_FLUX_HTML },
            { "pressure_stabilization", "" },
            { "soluteflux", "" },
            { "matching_osm_coef", "" },
            { "sbm point source", "" },
            { "solute point source", "" },
            { "fluid pressure", FLUID_PRESSURE_HTML },
            { "fluid viscous traction", FLUID_TRACTION_HTML },
            { "fluid mixture viscous traction", "" },
            { "fluid normal traction", FLUID_NORMAL_TRACTION_HTML },
            { "fluid normal velocity", FLUID_NORMAL_VELOCITY_HTML },
            { "fluid velocity", FLUID_VELOCITY_HTML },
            { "fluid resistance", FLUID_RESISTANCE_HTML },
            { "fluid RCR", "" },
            { "fluid tangential damping", "" },
            { "fluid tangential stabilization", FLUID_TANGENTIAL_STABILIZATION_HTML },
            { "fluid backflow stabilization", FLUID_BACKFLOW_STABILIZATION_HTML },
            { "fluid RC", "" },
            { "fluidP resistance", "" },
            { "fluid-FSI traction", FLUID_FSI_TRACTION_HTML },
            { "biphasic-FSI traction", BIPHASIC_FSI_TRACTION_HTML },
            { "multiphasic-FSI traction", "" },
            { "solute flux", SOLUTE_FLUX_HTML },
            { "solute backflow stabilization", "" },
            { "solute natural flux", "" },
            { "solute convective flow", "" },
            { "fluid heat flux", "" }
        }
    },
    // Initial Conditions
    {
        FEIC_ID,
        {
            { "init_dof", INITIAL_SECTION_HTML },
            { "velocity", INITIAL_SECTION_HTML },
            { "prestrain", THE_PRESTRAIN_INITIAL_CONDITION_HTML },
            { "initial fluid pressure", INITIAL_SECTION_HTML },
            { "initial concentration", INITIAL_SECTION_HTML },
            { "initial fluid dilatation", INITIAL_SECTION_HTML }
        }
    },
    // Contact Interfaces
    {
		FESURFACEINTERACTION_ID,
        {
            { "sliding-node-on-facet", SLIDING_INTERFACES_HTML },
            { "sliding-facet-on-facet", SLIDING_INTERFACES_HTML },
            { "sliding-elastic", SLIDING_INTERFACES_HTML },
            { "tied-node-on-facet", TIED_INTERFACES_HTML },
            { "tied-facet-on-facet", TIED_INTERFACES_HTML },
            { "tied-elastic", TIED_ELASTIC_INTERFACES_HTML },
            { "periodic boundary", "" },
            { "surface constraint", "" },
            { "sticky", STICKY_INTERFACES_HTML },
            { "mortar-sliding", "" },
            { "mortar-tied", "" },
            { "contact potential", "" },
            { "sliding2", SLIDING_INTERFACES_HTML },
            { "sliding-biphasic", BIPHASIC_CONTACT_HTML },
            { "sliding-biphasic-mixed", "" },
            { "tied-biphasic", TIED_BIPHASIC_INTERFACES_HTML },
            { "sliding-biphasic-solute", BIPHASIC_SOLUTE_AND_MULTIPHASIC_CONTACT_HTML },
            { "sliding-multiphasic",BIPHASIC_SOLUTE_AND_MULTIPHASIC_CONTACT_HTML"" },
            { "tied-multiphasic", TIED_MULTIPHASIC_INTERFACES_HTML },
            { "tied-fluid", "" },
            { "periodic boundary1O", "" },
            { "periodic boundary2O", "" }            
        }
    },
    // Constraints
    {
        FENLCONSTRAINT_ID,
        {
            { "point", "" },
            { "rigid joint", "" },
            { "generic rigid joint", "" },
            { "rigid spherical joint", RIGID_SPHERICAL_JOINT_HTML },
            { "rigid revolute joint", RIGID_REVOLUTE_JOINT_HTML },
            { "rigid prismatic joint", RIGID_PRISMATIC_JOINT_HTML },
            { "rigid cylindrical joint", RIGID_CYLINDRICAL_JOINT_HTML },
            { "rigid planar joint", RIGID_PLANAR_JOINT_HTML },
            { "rigid lock", RIGID_LOCK_JOINT_HTML },
            { "rigid spring", RIGID_SPRING_HTML },
            { "rigid damper", RIGID_DAMPER_HTML },
            { "rigid angular damper", RIGID_ANGULAR_DAMPER_HTML },
            { "rigid contractile force", RIGID_CONTRACTILE_FORCE_HTML },
            { "node distance", "" },
            { "prestrain", "" },
            { "in-situ stretch", "" },
            { "azimuth constraint", "" },
            { "node-on-node", "" }            
        }
    }
};

const char* GetHelpURL(int superID, string typeString)
{
    try
    {
        return URLs.at(superID).at(typeString).c_str();
    }
    catch(const std::out_of_range& e)
    {
        return "";
    }
    
}
