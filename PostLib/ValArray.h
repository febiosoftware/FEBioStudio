#pragma once
#include <vector>

//-----------------------------------------------------------------------------
class ValArray
{
public:
	ValArray() {}

	int itemSize(int n) const { return m_index[n + 1] - m_index[n]; }

	// append an item with n values
	void append(int n);

	float value(int item, int index) const { return m_data[m_index[item] + index]; }
	float& value(int item, int index) { return m_data[m_index[item] + index]; }

protected:
	std::vector<int>	m_index;
	std::vector<float>	m_data;
};

