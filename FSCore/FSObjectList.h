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
#include "FSObject.h"
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

class FSObjectList_ : public FSObject
{
public:
	FSObjectList_() {}
	~FSObjectList_() { Clear(); }

	void Clear()
	{
		for (size_t i = 0; i < m_obs.size(); ++i)
		{
			if (m_obs[i])
			{
				m_obs[i]->SetParent(nullptr);
				delete m_obs[i];
			}
		}
		if (m_obs.empty()==false) m_obs.clear();
	}

	bool IsEmpty() const
	{
		return m_obs.empty();
	}

	size_t Size() const
	{
		return m_obs.size();
	}

	void InsertChild(size_t pos, FSObject* po) override
	{
		po->SetParent(this);
		m_obs.insert(m_obs.begin() + pos, po);
	}

	int Find(FSObject* po)
	{
		for (int i = 0; i < m_obs.size(); ++i)
		{
			if (m_obs[i] == po) return i;
		}
		return -1;
	}

protected:
	FSObject* replace(int i, FSObject* po)
	{
		FSObject* old = m_obs[i];
		if (old) old->SetParent(nullptr);
		if (po) po->SetParent(this);
		m_obs[i] = po;
		return old;
	}

protected:
	std::vector<FSObject*>	m_obs;
};

template <typename T> class FSObjectList : public FSObjectList_
{
public:
	void Add(T* obj)
	{
		obj->SetParent(this);
		m_obs.push_back(obj);
	}

	size_t Remove(T* obj)
	{
		for (size_t i = 0; i < m_obs.size(); ++i)
		{
			if (m_obs[i] == obj)
			{
				obj->SetParent(nullptr);
				m_obs.erase(m_obs.begin() + i);
				return i;
			}
		}
		assert(false);
		return -1;
	}

	T* Remove(size_t pos)
	{
		if (pos < m_obs.size())
		{
			T* po = static_cast<T*>(m_obs[pos]);
			if (po) po->SetParent(nullptr);
			m_obs.erase(m_obs.begin() + pos);
			return po;
		}
		assert(false);
		return nullptr;
	}

	void Insert(size_t pos, T* obj)
	{
		InsertChild(pos, obj);
	}

	T* Replace(int i, T* po)
	{
		return dynamic_cast<T*>(replace(i, po));
	}

	T* Replace(T* pold, T* pnew)
	{
		for (int i = 0; i < Size(); ++i)
		{
			if (m_obs[i] == pold)
			{
				return Replace(i, pnew);
			}
		}
		return nullptr;
	}

	T* operator [] (size_t i) { return dynamic_cast<T*>(m_obs[i]); }
	const T* operator [] (size_t i) const { return dynamic_cast<T*>(m_obs[i]); }

	T* At(size_t i) { return dynamic_cast<T*>(m_obs[i]); }

	void Set(size_t pos, T* obj)
	{
		m_obs[pos] = obj;
	}

    void Move(size_t oldIndex, size_t newIndex)
    {
        if (oldIndex > newIndex)
            std::rotate(m_obs.rend() - oldIndex - 1, m_obs.rend() - oldIndex, m_obs.rend() - newIndex);
        else        
            std::rotate(m_obs.begin() + oldIndex, m_obs.begin() + oldIndex + 1, m_obs.begin() + newIndex + 1);
    }

	T* FindByName(const std::string& name)
	{
		for (int i = 0; i < m_obs.size(); ++i)
		{
			if (m_obs[i]->GetName() == name) return At(i);
		}
		return nullptr;
	}

protected:
	size_t RemoveChild(FSObject* po) override
	{
		for (size_t i = 0; i < m_obs.size(); ++i)
		{
			if (m_obs[i] == po)
			{
				m_obs.erase(m_obs.begin() + i);
				return i;
			}
		}
		assert(false);
		return -1;
	}
};

template <class T> void clearVector(FSObjectList<T>& v, std::function<bool(T*)> f)
{
	if (v.IsEmpty()) return;

	for (size_t i = 0; i < v.Size(); )
	{
		T* o = v[i];
		if (f(o))
		{
			v.Remove(o);
		}
		else i++;
	}
}
