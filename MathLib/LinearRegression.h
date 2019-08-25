#pragma once
#include <vector>
using namespace std;

bool LinearRegression(const vector<pair<double, double> >& data, pair<double, double>& res);

bool NonlinearRegression(const vector<pair<double, double> >& data, vector<double>& res, int func);
