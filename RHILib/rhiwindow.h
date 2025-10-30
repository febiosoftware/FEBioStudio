// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause
#pragma once
#include <QWindow>
#include <QOffscreenSurface>
#include <rhi/qrhi.h>
#include <GLLib/GLCamera.h>
#include "rhiMesh.h"

class RhiWindow : public QWindow
{
public:
	enum RenderMode {
		STATIC,
		DYNAMIC
	};

public:
    RhiWindow();
    QString graphicsApiName() const;
    void releaseSwapChain();

	void setRenderMode(RenderMode rm);

	static void InitRHI(QRhi::Implementation graphicsApi);

protected:
    virtual void customInit() = 0;
    virtual void customRender() = 0;

	// This is called after each frame is finished (i.e. after called QRhi::endFrame)
	// Use this to update dynamic data for the next frame
	virtual void onFrameFinished() {}

    // destruction order matters to a certain degree: the fallbackSurface must
    // outlive the rhi, the rhi must outlive all other resources.  The resources
    // need no special order when destroying.
#if QT_CONFIG(opengl)
    std::unique_ptr<QOffscreenSurface> m_fallbackSurface;
#endif
    std::unique_ptr<QRhi> m_rhi;
    std::unique_ptr<QRhiSwapChain> m_sc;
    std::unique_ptr<QRhiRenderBuffer> m_ds;
    std::unique_ptr<QRhiRenderPassDescriptor> m_rp;
    bool m_hasSwapChain = false;
	unsigned int sampleCount = 4;

protected:
    void init();
    void resizeSwapChain();
    void render();

    void exposeEvent(QExposeEvent *) override;
    bool event(QEvent *) override;

private:
    bool m_initialized = false;
    bool m_notExposed = false;
    bool m_newlyExposed = false;

	RenderMode m_renderMode = RenderMode::STATIC;

    static QRhi::Implementation m_graphicsApi;
	static bool rhi_initialized;
};
