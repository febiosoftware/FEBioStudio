#pragma once

#include "GLObject.h"
#include "GLCamera.h"

namespace Post {
//-----------------------------------------------------------------------------
// This class stores viewing information
class CGView : public CGLObject
{
public:
	CGView();
	~CGView();

	Post::CGLCamera& GetCamera() { return m_cam; }

	void Reset();

	int CameraKeys() { return (int) m_key.size(); }

	Post::GLCameraTransform& GetKey(int i) { return *m_key[i]; }
	Post::GLCameraTransform& GetCurrentKey() { return *m_key[m_nkey]; }
	void SetCurrentKey(Post::GLCameraTransform* pkey);
	void SetCurrentKey(int i);

	void AddCameraKey(Post::GLCameraTransform& t);

	void DeleteKey(Post::GLCameraTransform* pt);

	void DeleteAllKeys();

	void PrevKey();
	void NextKey();

protected:
	Post::CGLCamera m_cam;	//!< current camera

	vector<Post::GLCameraTransform*>	m_key;	//!< stored camera transformations
	int							m_nkey;	//!< current key
};
}
