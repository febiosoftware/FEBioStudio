#pragma once
#include "color.h"
#include <string.h>

// rendering modes
#define RENDER_MODE_DEFAULT	0
#define RENDER_MODE_WIRE	1
#define RENDER_MODE_SOLID	2

#define RENDER_TRANS_CONSTANT	0
#define RENDER_TRANS_WEIGHTED	1

namespace Post {

//-----------------------------------------------------------------------------
// Class that describes the visual appearance of a material. 
class FEMaterial
{
public:
	enum {MAX_NAME = 64};

public:
	FEMaterial();

	const char* GetName();
	void SetName(const char* szname);

	bool visible() const { return bvisible; }
	void hide() { bvisible = false; }
	void show() { bvisible = true; }

	bool enabled() const { return benable; }
	void enable() { benable = true; }
	void disable() { benable = false; }

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
	char	m_szname[MAX_NAME];		// name of material
};
}
