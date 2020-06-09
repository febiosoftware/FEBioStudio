#pragma once
#include "GLCamera.h"

//-----------------------------------------------------------------------------
// This class stores viewing information
class CGView : public FSObject
{
public:
	CGView();
	~CGView();

	CGLCamera& GetCamera() { return m_cam; }

	void Reset();

	int CameraKeys() { return (int) m_key.size(); }

	GLCameraTransform& GetKey(int i) { return *m_key[i]; }
	GLCameraTransform& GetCurrentKey() { return *m_key[m_nkey]; }
	void SetCurrentKey(GLCameraTransform* pkey);
	void SetCurrentKey(int i);

	void AddCameraKey(GLCameraTransform& t);

	void DeleteKey(GLCameraTransform* pt);

	void DeleteAllKeys();

	void PrevKey();
	void NextKey();

	bool OrhographicProjection() { return m_bortho; }

	double GetFOV() { return m_fov; }
	double GetAspectRatio() { return m_ar; }
	double GetNearPlane() { return m_fnear; }
	double GetFarPlane() { return m_ffar; }

public:
	bool	m_bortho;		// orthographic mode
	double	m_fnear;
	double	m_ffar;
	double	m_fov;
	double	m_ar;

protected:
	CGLCamera m_cam;	//!< current camera

	vector<GLCameraTransform*>	m_key;	//!< stored camera transformations
	int							m_nkey;	//!< current key
};
