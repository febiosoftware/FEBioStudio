#pragma once
#include "FSObject.h"
#include <vector>

class FSObjectList_ : public FSObject
{
public:
	FSObjectList_() {}
	~FSObjectList_() { Clear(); }

	void Clear()
	{
		for (size_t i = 0; i < m_obs.size(); ++i)
		{
			m_obs[i]->SetParent(nullptr);
			delete m_obs[i];
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

	void Insert(size_t pos, T* obj)
	{
		InsertChild(pos, obj);
	}

	T* operator [] (size_t i) { return dynamic_cast<T*>(m_obs[i]); }
	const T* operator [] (size_t i) const { return dynamic_cast<T*>(m_obs[i]); }

	void Set(size_t pos, T* obj)
	{
		m_obs[pos] = obj;
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
