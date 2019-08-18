#pragma once
#include <GeomLib/GTransform.h>
#include <vector>
using namespace std;

class GObject;


class GICPRegistration
{
public:
	GICPRegistration();

	// returns the transform from registring source to target
	GTransform Register(GObject* ptrg, GObject* psrc, const double tol = 0.001, const int maxIter = 100);

private:
	void ClosestPointSet(const vector<vec3d>& X, const vector<vec3d>& P, vector<vec3d>& Y);
	vec3d CenterOfMass(const vector<vec3d>& S);
	GTransform Register(const vector<vec3d>& P0, const vector<vec3d>& Y, double* err);
	void ApplyTransform(const vector<vec3d>& P0, const GTransform& Q, vector<vec3d>& P);
};
