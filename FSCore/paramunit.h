#pragma once
// units are defined as strings that use the following
// characters to represent the physical quantities that
// define the SI base units.
// (Note that the symbols for time and temperature differ
// from the conventional SI dimension symbols)
//
// L = length
// M = mass
// t = time
// T = temperature
// l = electric current
// n = substance
//
// In addition, the following symbols for derived units are also defined:
//
// F = force
// P = pressure
// E = energy
// W = power
// d = angles in degrees
// r = angles in radians
//
// or use one of these predefined constants
#define UNIT_NONE	""
#define UNIT_LENGTH "L"
#define UNIT_MASS   "M"
#define UNIT_TIME   "t"
#define UNIT_TEMPERATURE "T"
#define UNIT_CURRENT "I"
#define UNIT_SUBSTANCE	"n"
#define UNIT_FORCE "F"
#define UNIT_PRESSURE "P"
#define UNIT_ENERGY	"E"
#define UNIT_POWER "W"
#define UNIT_VOLTAGE "V"
#define UNIT_CONCENTRATION "c"

#define UNIT_DEGREE "d"
#define UNIT_RADIAN	"r"

#define UNIT_AREA   "L^2"
#define UNIT_VOLUME "L^3"
#define UNIT_VELOCITY "L/t"
#define UNIT_ACCELERATION "L/t^2"
#define UNIT_ANGULAR_VELOCITY "r/t"
#define UNIT_DENSITY "M/L^3"
#define UNIT_PERMEABILITY	"L^4/F.t"
#define UNIT_DIFFUSIVITY  "L^2/t"
#define UNIT_MOLAR_MASS "M/n"
#define UNIT_MOLAR_VOLUME "L^3/n"
#define UNIT_GAS_CONSTANT "E/n.T"
#define UNIT_FARADAY_CONSTANT "I.t/n"
#define UNIT_VISCOSITY "P.t"
#define UNIT_FILTRATION "L^2/F.t"
#define UNIT_STIFFNESS "F/L"
