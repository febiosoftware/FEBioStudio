#include "stdafx.h"
#include "ValArray.h"

void ValArray::append(int items)
{
	if (m_index.empty()) m_index.push_back(0);
	m_index.push_back((int)m_data.size() + items);
	for (int i = 0; i<items; ++i) m_data.push_back(0.f);
}
