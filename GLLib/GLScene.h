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
#include <FSCore/box.h>
#include "GLRenderStats.h"
#include "GLGrid.h"
#include "GLCamera.h"
#include "GLRenderEngine.h"
#include <vector>

class GLContext;
class GLScene;
class QPainter;

// tag structure
struct GLTAG
{
	char	sztag[64];	// name of tag
	float	wx, wy;		// window coordinates for tag
	vec3d	r;			// world coordinates of tag
	GLColor	c;			// tag color
};

class GLSceneItem;

typedef std::vector<GLSceneItem*> GLItemList;
typedef std::vector<GLSceneItem*>::const_iterator ConstGLItemIterator;
typedef std::vector<GLSceneItem*>::iterator GLItemIterator;

class GLSceneItem
{
public:
	GLSceneItem() {}
	virtual ~GLSceneItem() {}

	virtual void clear();

	virtual void render(GLRenderEngine& re, GLContext& rc);

public:
	size_t children() const { return m_children.size(); }

	GLSceneItem* child(size_t i) { return m_children[i]; }

	void addChild(GLSceneItem* item) { if (item) m_children.push_back(item); }

	GLItemIterator begin() { return m_children.begin(); }
	GLItemIterator end() { return m_children.end(); }

	ConstGLItemIterator begin() const { return m_children.begin(); }
	ConstGLItemIterator end() const { return m_children.end(); }

private:
	GLSceneItem(const GLSceneItem&) = delete;
	GLSceneItem& operator = (const GLSceneItem&) = delete;

private:
	GLItemList m_children;
};

struct GLSceneStats
{
	int updates = 0;
	int rebuilds = 0;
};

class GLScene
{
public:
	GLScene();
	virtual ~GLScene();

	virtual void Update();

	GLSceneStats GetStats() const { return stats; }

	// Render the 3D scene
	virtual void Render(GLRenderEngine& engine, GLContext& rc);

	virtual BOX GetBoundingBox() { return BOX(); }

	GLCamera& GetCamera() { return m_cam; }

	void PositionCameraInScene(GLRenderEngine& re);

	void SetupProjection(GLRenderEngine& re);

public:
	GLGrid& GetGrid() { return m_grid; }
	double GetGridScale() { return m_grid.GetScale(); }

	void SetEnvironmentMap(const CRGBAImage& img) { m_envMap = img; }

	void ActivateEnvironmentMap(GLRenderEngine& re);
	void DeactivateEnvironmentMap(GLRenderEngine& re);

public:
	void AddTag(const GLTAG& tag) { m_tags.push_back(tag); }
	void AddTags(const std::vector<GLTAG>& tag) { m_tags.insert(m_tags.end(), tag.begin(), tag.end()); }
	void ClearTags() { m_tags.clear(); }

	size_t Tags() const { return m_tags.size(); }
	GLTAG& Tag(size_t i) { return m_tags[i]; }

public:
	GLItemIterator begin() { return m_root.begin(); }
	GLItemIterator end() { return m_root.end(); }

	ConstGLItemIterator begin() const { return m_root.begin(); }
	ConstGLItemIterator end() const { return m_root.end(); }

	void addItem(GLSceneItem* item) { if (item) m_root.addChild(item); }

	size_t items() const { return m_root.children(); }

	void clear();

	template <typename T>
	std::vector<T*> GetItemsOfType()
	{
		std::vector<T*> items;

		std::stack<GLSceneItem*> s;
		s.push(&m_root);
		while (!s.empty()) {
			GLSceneItem* item = s.top(); s.pop();

			if (T* casted = dynamic_cast<T*>(item)) {
				items.push_back(casted);
			}

			for (GLSceneItem* child : *item) {
				s.push(child);
			}
		}
		return items;
	}

protected:
	GLCamera m_cam;	//!< camera
	GLGrid	m_grid;	//!< the grid object
	BOX m_box;

	unsigned int m_envtex;	// enironment texture ID
	CRGBAImage m_envMap; // the texture to use

	std::vector<GLTAG> m_tags;

	GLSceneItem m_root;

	GLSceneStats stats; // number of calls to Update
};
