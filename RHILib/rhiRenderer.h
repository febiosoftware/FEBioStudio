/*This file is part of the FEBio Studio source code and is licensed under the MIT license
listed below.

See Copyright-FEBio-Studio.txt for details.

Copyright (c) 2025 University of Utah, The Trustees of Columbia University in
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
#include <GLLib/GLRenderEngine.h>
#include <rhi/qrhi.h>
#include "rhiMesh.h"
#include "rhiRenderPass.h"
#include "rhiPointRenderPass.h"
#include "rhiLineRenderPass.h"
#include "rhiSolidRenderPass.h"
#include "rhiVolumeRenderPass.h"
#include "rhiOverlay.h"
#include "rhiCanvas.h"
#include <GLLib/GLMeshBuilder.h>
#include <chrono>
using namespace std::chrono;
using dseconds = duration<double>;

class GlobalUniformBlock
{
public:
	GlobalUniformBlock() {}
	void create(QRhi* rhi);

	QRhiBuffer* get() { return m_ubuf.get(); }

	void update(QRhiResourceUpdateBatch* u);

public:
	void setLightPosition(const vec3f& lp);
	void setSpecularColor(GLColor c);
	void setClipPlane(const float f[4]);

private:
	rhi::UniformBlock m_ub;
	std::unique_ptr<QRhiBuffer> m_ubuf;
};

class rhiRenderer : public GLRenderEngine
{
	struct TimingInfo
	{
		time_point<steady_clock> m_tic;
		time_point<steady_clock> m_toc;
		double m_fps = 0, m_fpsMin = 0, m_fpsMax = 0;
		unsigned int m_frame = 0;
		double totalSec = 0;

		void update();
	};

public:
	rhiRenderer(QRhi* rhi, QRhiSwapChain* sc, QRhiRenderPassDescriptor* rp);
	~rhiRenderer();

	void init();

	void start() override;
	void finish() override;

	// clear all cached resources
	void clearCache();

	// clear cached resources that were not used in the last frame
	void clearUnusedCache();

	void deleteCachedMesh(GLMesh* gm) override;

	QSize pixelSize() const;

	GLRenderStats GetRenderStats() const override;

public:

	void setProjection(double fov, double fnear, double ffar) override;
	void setOrthoProjection(double left, double right, double bottom, double top, double zNear, double zFar) override;

	void resetTransform() override;
	void pushTransform() override;
	void popTransform() override;
	void translate(const vec3d& r) override;
	void rotate(const quatd& rot) override;
	void rotate(double deg, double x, double y, double z) override;
	void scale(double x, double, double z) override;

	void setLightPosition(unsigned int n, const vec3f& lp) override;
	void setLightSpecularColor(unsigned int lightIndex, const GLColor& col);

	void setBackgroundColor(const GLColor& c) override;

	void setMaterial(GLMaterial::Type matType, GLColor c, GLMaterial::DiffuseMap map, bool frontOnly) override;

	void setColor(GLColor c) override;
	void setMaterial(const GLMaterial& mat) override;

	void renderGMesh(const GLMesh& mesh, bool cacheMesh = true) override;
	void renderGMesh(const GLMesh& mesh, int surfId, bool cacheMesh = true) override;

	void renderGMeshEdges(const GLMesh& mesh, bool cacheMesh = true) override;
	void renderGMeshNodes(const GLMesh& mesh, bool cacheMesh = true) override;

	void setTexture(GLTexture1D& tex);
	void setTexture(GLTexture3D& tex);

	void setClipPlane(unsigned int n, const double* v) override;
	void enableClipPlane(unsigned int n) override;
	void disableClipPlane(unsigned int n) override;

public: // immediate mode rendering
	void beginShape() override;
	void endShape() override;

	void begin(PrimitiveType prim) override;
	void end() override;

	void vertex(const vec3d& r) override;
	void normal(const vec3d& n) override;
	void texCoord1d(double t) override;
	void texCoord2d(double r, double s) override;

public:
	void setOverlayImage(const QImage& img);
	void useOverlayImage(bool b);

	double getFPS() const { return timing.m_fps; }

	void setDPR(double dpr) { m_dpr = dpr; }

	void setTriadInfo(const QMatrix4x4& m, QRhiViewport vp);

private:
	QRhi* m_rhi;
	QRhiSwapChain* m_sc;
	QRhiRenderPassDescriptor* m_rp;

	// timing info
	TimingInfo timing;

	// 3D render passes
	std::unique_ptr<LineRenderPass> m_linePass;
	std::unique_ptr<LineRenderPass> m_lineOverlayPass;
	std::unique_ptr<PointRenderPass> m_pointPass;
	std::unique_ptr<PointRenderPass> m_pointOverlayPass;
	std::unique_ptr<TwoPassSolidRenderPass> m_solidPass;
	std::unique_ptr<SolidRenderPass> m_solidOverlayPass;
	std::unique_ptr<VolumeRenderPass> m_volumeRenderPass;

	// 2D render passes
	std::unique_ptr<OverlayRenderPass> m_overlay2DPass;
	std::unique_ptr<CanvasRenderPass> m_canvasPass;

	// global resources
	GlobalUniformBlock m_global;
	rhi::Texture m_tex1D;
	rhi::SharedResources m_sharedResources;

	// used for submitting updates in init.
	QRhiResourceUpdateBatch* m_initialUpdates = nullptr;

	// matrix stuff
	std::array<float, 4> m_viewport{ 0.f };
	QMatrix4x4 m_projMatrix;
	QMatrix4x4 m_modelViewMatrix;
	std::stack<QMatrix4x4> m_transformStack;

	// overlay flag
	bool m_useOverlay = false;

	double m_dpr = 1;

private:
	GLMeshBuilder mb;
	bool buildingShape = false;

private:
	// variables used when creating meshes
	vec3f m_light;
	GLColor m_lightSpecular;
	QColor m_bgColor;
	GLMaterial m_currentMat;
	bool m_clipEnabled = false;
	float clipPlane[4];
};
