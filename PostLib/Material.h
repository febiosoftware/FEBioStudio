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
#include <FSCore/color.h>
#include <string>

// rendering modes
#define RENDER_MODE_DEFAULT	0
#define RENDER_MODE_SOLID	1
#define RENDER_MODE_WIRE	2

enum TransparencyMode {
	RENDER_TRANS_CONSTANT,
	RENDER_TRANS_NORMAL_WEIGHTED,
	RENDER_TRANS_VALUE_WEIGHTED,
};

namespace Post {

//-----------------------------------------------------------------------------
// Class that describes the visual appearance of a material. 
class Material
{
public:
	Material();

	const std::string& GetName() const;
	void SetName(const std::string& s);

	bool visible() const { return bvisible; }
	void hide() { bvisible = false; }
	void show() { bvisible = true; }

	bool enabled() const { return benable; }
	void enable() { benable = true; }
	void disable() { benable = false; }

	void setColor(uint8_t r, uint8_t g, uint8_t b) { diffuse = ambient = GLColor(r,g,b); }

public:
	GLColor	diffuse;		// diffuse material color
	GLColor	ambient;		// ambient material color
	GLColor	specular;		// specular material color
	GLColor emission;		// emission material color
	GLColor meshcol;		// mesh color
	float	shininess;		// shininess [0..1]
	float	transparency;	// transparency [0..1]
	bool	benable;		// material enabled or not
	bool	bvisible;		// material visible or not
	bool	bmesh;			// show mesh on model or not
	bool	bcast_shadows;	// cast shadows or not
	bool	bclip;			// allow the material to be clipped
	int		m_nrender;		// render mode
	int		m_ntransmode;	// transparency mode

protected:
	std::string m_name;
};
}
