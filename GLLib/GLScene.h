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

class GLSceneItem
{
public:
	GLSceneItem() {}
	virtual ~GLSceneItem() {}

	virtual void render(GLRenderEngine& re, GLContext& rc) = 0;
};

typedef std::vector<GLSceneItem*> GLItemList;
typedef std::vector<GLSceneItem*>::const_iterator ConstGLItemIterator;
typedef std::vector<GLSceneItem*>::iterator GLItemIterator;

class GLCompositeSceneItem : public GLSceneItem
{
public:
	GLCompositeSceneItem();
	virtual ~GLCompositeSceneItem();

	void render(GLRenderEngine& re, GLContext& rc) override;

public:
	void addItem(GLSceneItem* item) { if (item) m_items.push_back(item); }

private:
	GLItemList m_items;
};

class GLScene
{
public:
	GLScene();
	virtual ~GLScene();

	virtual void Update();

	// Render the 3D scene
	virtual void Render(GLRenderEngine& engine, GLContext& rc);

	// Render on the 2D canvas
//	virtual void RenderCanvas(QPainter& painter, GLContext& rc) {}

	virtual BOX GetBoundingBox() { return BOX(); }

	GLCamera& GetCamera() { return m_cam; }

	void PositionCameraInScene(GLRenderEngine& re);

	void SetupProjection(GLRenderEngine& re);

public:
	GLGrid& GetGrid() { return m_grid; }
	double GetGridScale() { return m_grid.GetScale(); }

	void SetEnvironmentMap(const std::string& filename) { m_envMap = filename; }

	void ActivateEnvironmentMap(GLRenderEngine& re);
	void DeactivateEnvironmentMap(GLRenderEngine& re);
	void LoadEnvironmentMap(GLRenderEngine& re);

public:
	void AddTag(const GLTAG& tag) { m_tags.push_back(tag); }
	void AddTags(const std::vector<GLTAG>& tag) { m_tags.insert(m_tags.end(), tag.begin(), tag.end()); }
	void ClearTags() { m_tags.clear(); }

	size_t Tags() const { return m_tags.size(); }
	GLTAG& Tag(size_t i) { return m_tags[i]; }

public:
	GLItemIterator begin() { return m_Items.begin(); }
	GLItemIterator end() { return m_Items.end(); }

	ConstGLItemIterator begin() const { return m_Items.begin(); }
	ConstGLItemIterator end() const { return m_Items.end(); }

	void addItem(GLSceneItem* item) { if (item) m_Items.push_back(item); }

	size_t items() const { return m_Items.size(); }

	void clear();

protected:
	GLCamera m_cam;	//!< camera
	GLGrid	m_grid;	//!< the grid object
	BOX m_box;

	unsigned int	m_envtex;	// enironment texture ID
	std::string		m_envMap; // file name used for environment mapping 

	std::vector<GLTAG> m_tags;

	GLItemList m_Items;
};
