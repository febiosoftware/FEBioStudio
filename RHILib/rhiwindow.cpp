// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

#include "rhiwindow.h"
#include <QPlatformSurfaceEvent>

bool RhiWindow::rhi_initialized = false;

QRhi::Implementation RhiWindow::m_graphicsApi = QRhi::Null;

#ifndef Q_OS_MACOS
#if QT_CONFIG(vulkan)
static QVulkanInstance vulkanInst;
#endif
#endif

void RhiWindow::InitRHI(QRhi::Implementation api)
{
	// see if we're already initialized
	if (rhi_initialized) return;

#ifndef Q_OS_MACOS
#if QT_CONFIG(vulkan)
	if (api == QRhi::Vulkan) {
		// Request validation, if available. This is completely optional
		// and has a performance impact, and should be avoided in production use.
		vulkanInst.setLayers({ "VK_LAYER_KHRONOS_validation" });
		// Play nice with QRhi.
		vulkanInst.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
		if (!vulkanInst.create()) {
			qWarning("Failed to create Vulkan instance, switching to OpenGL");
			api = QRhi::OpenGLES2;
		}
	}
#endif
#endif

	// For OpenGL, to ensure there is a depth/stencil buffer for the window.
	 // With other APIs this is under the application's control (QRhiRenderBuffer etc.)
	 // and so no special setup is needed for those.
	if (api == QRhi::OpenGLES2)
	{
		QSurfaceFormat fmt;
		fmt.setDepthBufferSize(24);
		fmt.setStencilBufferSize(8);
		fmt.setSamples(4);
		// Special case macOS to allow using OpenGL there.
		// (the default Metal is the recommended approach, though)
		// gl_VertexID is a GLSL 130 feature, and so the default OpenGL 2.1 context
		// we get on macOS is not sufficient.
#ifdef Q_OS_MACOS
		fmt.setVersion(4, 1);
		fmt.setProfile(QSurfaceFormat::CoreProfile);
#endif
		QSurfaceFormat::setDefaultFormat(fmt);
	}

	// done with initialization
	rhi_initialized = true;
	m_graphicsApi = api;
}

RhiWindow::RhiWindow()
{
    switch (m_graphicsApi) {
    case QRhi::OpenGLES2:
        setSurfaceType(OpenGLSurface);
        break;
    case QRhi::Vulkan:
        setSurfaceType(VulkanSurface);
        break;
    case QRhi::D3D11:
    case QRhi::D3D12:
        setSurfaceType(Direct3DSurface);
        break;
    case QRhi::Metal:
        setSurfaceType(MetalSurface);
        break;
    case QRhi::Null:
        break; // RasterSurface
    }

#ifndef Q_OS_MACOS
#if QT_CONFIG(vulkan)
	if (m_graphicsApi == QRhi::Vulkan)
		setVulkanInstance(&vulkanInst);
#endif
#endif

	// choose sample count for MSAA
	sampleCount = 4;
}

QString RhiWindow::graphicsApiName() const
{
    switch (m_graphicsApi) {
    case QRhi::Null:
        return QLatin1String("Null (no output)");
    case QRhi::OpenGLES2:
        return QLatin1String("OpenGL");
    case QRhi::Vulkan:
        return QLatin1String("Vulkan");
    case QRhi::D3D11:
        return QLatin1String("Direct3D 11");
    case QRhi::D3D12:
        return QLatin1String("Direct3D 12");
    case QRhi::Metal:
        return QLatin1String("Metal");
    }
    return QString();
}

void RhiWindow::exposeEvent(QExposeEvent *)
{
    // initialize and start rendering when the window becomes usable for graphics purposes
    if (isExposed() && !m_initialized) {
        init();
        resizeSwapChain();
        m_initialized = true;
    }

    const QSize surfaceSize = m_hasSwapChain ? m_sc->surfacePixelSize() : QSize();

    // stop pushing frames when not exposed (or size is 0)
    if ((!isExposed() || (m_hasSwapChain && surfaceSize.isEmpty())) && m_initialized && !m_notExposed)
        m_notExposed = true;

    // Continue when exposed again and the surface has a valid size. Note that
    // surfaceSize can be (0, 0) even though size() reports a valid one, hence
    // trusting surfacePixelSize() and not QWindow.
    if (isExposed() && m_initialized && m_notExposed && !surfaceSize.isEmpty()) {
        m_notExposed = false;
        m_newlyExposed = true;
    }

    // always render a frame on exposeEvent() (when exposed) in order to update
    // immediately on window resize.
    if (isExposed() && !surfaceSize.isEmpty())
        render();
}

bool RhiWindow::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::UpdateRequest:
        render();
        break;

    case QEvent::PlatformSurface:
        // this is the proper time to tear down the swapchain (while the native window and surface are still around)
        if (static_cast<QPlatformSurfaceEvent *>(e)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
            releaseSwapChain();
        break;

    default:
        break;
    }

    return QWindow::event(e);
}

void RhiWindow::init()
{
    if (m_graphicsApi == QRhi::Null) {
        QRhiNullInitParams params;
        m_rhi.reset(QRhi::create(QRhi::Null, &params));
    }

#if QT_CONFIG(opengl)
    if (m_graphicsApi == QRhi::OpenGLES2) {
        m_fallbackSurface.reset(QRhiGles2InitParams::newFallbackSurface());
        QRhiGles2InitParams params;
        params.fallbackSurface = m_fallbackSurface.get();
        params.window = this;
        m_rhi.reset(QRhi::create(QRhi::OpenGLES2, &params));
    }
#endif

#ifndef Q_OS_MACOS
#if QT_CONFIG(vulkan)
    if (m_graphicsApi == QRhi::Vulkan) {
        QRhiVulkanInitParams params;
        params.inst = vulkanInstance();
        params.window = this;
        m_rhi.reset(QRhi::create(QRhi::Vulkan, &params));
    }
#endif
#endif

#ifdef Q_OS_WIN
    if (m_graphicsApi == QRhi::D3D11) {
        QRhiD3D11InitParams params;
        // Enable the debug layer, if available. This is optional
        // and should be avoided in production builds.
        params.enableDebugLayer = true;
        m_rhi.reset(QRhi::create(QRhi::D3D11, &params));
    } else if (m_graphicsApi == QRhi::D3D12) {
        QRhiD3D12InitParams params;
        // Enable the debug layer, if available. This is optional
        // and should be avoided in production builds.
        params.enableDebugLayer = true;
        m_rhi.reset(QRhi::create(QRhi::D3D12, &params));
    }
#endif

#if defined(Q_OS_MACOS) || defined(Q_OS_IOS)
    if (m_graphicsApi == QRhi::Metal) {
        QRhiMetalInitParams params;
        m_rhi.reset(QRhi::create(QRhi::Metal, &params));
    }
#endif

    if (!m_rhi)
        qFatal("Failed to create RHI backend");

    m_sc.reset(m_rhi->newSwapChain());
	m_sc->setSampleCount(sampleCount);
	m_sc->setFlags(QRhiSwapChain::UsedAsTransferSource);

    m_ds.reset(m_rhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil,
                                      QSize(), // no need to set the size here, due to UsedWithSwapChainOnly
                                      sampleCount,
                                      QRhiRenderBuffer::UsedWithSwapChainOnly));
    m_sc->setDepthStencil(m_ds.get());
    m_sc->setWindow(this);

    m_rp.reset(m_sc->newCompatibleRenderPassDescriptor());
    m_sc->setRenderPassDescriptor(m_rp.get());

    customInit();
}

void RhiWindow::resizeSwapChain()
{
    m_hasSwapChain = m_sc->createOrResize(); // also handles m_ds
}

void RhiWindow::releaseSwapChain()
{
    if (m_hasSwapChain) {
        m_hasSwapChain = false;
        m_sc->destroy();
    }
}

void RhiWindow::setRenderMode(RenderMode rm)
{
	if (rm != m_renderMode)
	{
		m_renderMode = rm;
		if (rm == DYNAMIC) requestUpdate();
	}
}

void RhiWindow::render()
{
    if (!m_hasSwapChain || m_notExposed)
        return;

	// If the window got resized or newly exposed, resize the swapchain. (the
    // newly-exposed case is not actually required by some platforms, but is
    // here for robustness and portability)
    //
    // This (exposeEvent + the logic here) is the only safe way to perform
    // resize handling. Note the usage of the RHI's surfacePixelSize(), and
    // never QWindow::size(). (the two may or may not be the same under the hood,
    // depending on the backend and platform)
    //
    if (m_sc->currentPixelSize() != m_sc->surfacePixelSize() || m_newlyExposed) {
        resizeSwapChain();
        if (!m_hasSwapChain)
            return;
        m_newlyExposed = false;
    }

	QRhi::FrameOpResult result = m_rhi->beginFrame(m_sc.get());
    if (result == QRhi::FrameOpSwapChainOutOfDate) {
        resizeSwapChain();
        if (!m_hasSwapChain)
            return;
        result = m_rhi->beginFrame(m_sc.get());
    }
    if (result != QRhi::FrameOpSuccess) {
        qWarning("beginFrame failed with %d, will retry", result);
        requestUpdate();
        return;
    }

    customRender();

	m_rhi->endFrame(m_sc.get());

	// do cleanup
	onFrameFinished();

    // Always request the next frame via requestUpdate(). On some platforms this is backed
    // by a platform-specific solution, e.g. CVDisplayLink on macOS, which is potentially
    // more efficient than a timer, queued metacalls, etc.
	if (m_renderMode == RenderMode::DYNAMIC)
		requestUpdate();
}
