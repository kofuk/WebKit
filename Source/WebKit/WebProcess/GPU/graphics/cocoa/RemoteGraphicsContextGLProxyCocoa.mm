/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#import "config.h"
#import "RemoteGraphicsContextGLProxy.h"

#if ENABLE(GPU_PROCESS) && ENABLE(WEBGL)
#import "GPUConnectionToWebProcess.h"
#import "GPUProcessConnection.h"
#import "RemoteGraphicsContextGLMessages.h"
#import "WebProcess.h"
#import <WebCore/CVUtilities.h>
#import <WebCore/GraphicsContextCG.h>
#import <WebCore/GraphicsContextGLIOSurfaceSwapChain.h>
#import <WebCore/IOSurface.h>
#import <WebCore/MediaSampleAVFObjC.h>
#import <WebCore/WebGLLayer.h>
#import <wtf/BlockObjCExceptions.h>

namespace WebKit {

namespace {

class RemoteGraphicsContextGLProxyCocoa final : public RemoteGraphicsContextGLProxy {
public:
    bool isValid() const { return m_webGLLayer; }
    WebCore::IOSurface* displayBuffer() const { return m_displayBuffer.get(); }

    // RemoteGraphicsContextGLProxy overrides.
    PlatformLayer* platformLayer() const final { return m_webGLLayer.get(); }
    void prepareForDisplay() final;
#if ENABLE(VIDEO) && USE(AVFOUNDATION)
    WebCore::GraphicsContextGLCV* asCV() final { return nullptr; }
#endif
#if ENABLE(MEDIA_STREAM)
    RefPtr<WebCore::MediaSample> paintCompositedResultsToMediaSample() final;
#endif
private:
    RemoteGraphicsContextGLProxyCocoa(GPUProcessConnection&, const WebCore::GraphicsContextGLAttributes&, RenderingBackendIdentifier);
    RetainPtr<WebGLLayer> m_webGLLayer;
    std::unique_ptr<WebCore::IOSurface> m_displayBuffer;
    friend class RemoteGraphicsContextGLProxy;
};

RemoteGraphicsContextGLProxyCocoa::RemoteGraphicsContextGLProxyCocoa(GPUProcessConnection& gpuProcessConnection, const WebCore::GraphicsContextGLAttributes& attributes, RenderingBackendIdentifier renderingBackend)
    : RemoteGraphicsContextGLProxy(gpuProcessConnection, attributes, renderingBackend)
{
    auto attrs = contextAttributes();
    BEGIN_BLOCK_OBJC_EXCEPTIONS
    m_webGLLayer = adoptNS([[WebGLLayer alloc] initWithDevicePixelRatio:attrs.devicePixelRatio contentsOpaque:!attrs.alpha]);
#ifndef NDEBUG
    [m_webGLLayer setName:@"WebGL Layer"];
#endif
    END_BLOCK_OBJC_EXCEPTIONS
}

#if ENABLE(MEDIA_STREAM)
RefPtr<WebCore::MediaSample> RemoteGraphicsContextGLProxyCocoa::paintCompositedResultsToMediaSample()
{
    if (!m_displayBuffer)
        return nullptr;
    auto pixelBuffer = WebCore::createCVPixelBuffer(m_displayBuffer->surface());
    if (!pixelBuffer)
        return nullptr;
    return WebCore::MediaSampleAVFObjC::createImageSample(WTFMove(*pixelBuffer), WebCore::MediaSampleAVFObjC::VideoRotation::UpsideDown, true);
}
#endif

void RemoteGraphicsContextGLProxyCocoa::prepareForDisplay()
{
    if (isContextLost())
        return;
    MachSendRight displayBufferSendRight;
    auto sendResult = sendSync(Messages::RemoteGraphicsContextGL::PrepareForDisplay(), Messages::RemoteGraphicsContextGL::PrepareForDisplay::Reply(displayBufferSendRight));
    if (!sendResult) {
        markContextLost();
        return;
    }
    if (!displayBufferSendRight)
        return;
    auto displayBuffer = WebCore::IOSurface::createFromSendRight(WTFMove(displayBufferSendRight), WebCore::DestinationColorSpace::SRGB());
    if (!displayBuffer) {
        markContextLost();
        return;
    }
    m_displayBuffer = WTFMove(displayBuffer);
    BEGIN_BLOCK_OBJC_EXCEPTIONS
    [m_webGLLayer setContents:m_displayBuffer->asLayerContents()];
    END_BLOCK_OBJC_EXCEPTIONS
    markLayerComposited();
}

}

RefPtr<RemoteGraphicsContextGLProxy> RemoteGraphicsContextGLProxy::create(const WebCore::GraphicsContextGLAttributes& attributes, RenderingBackendIdentifier renderingBackend)
{
    auto context = adoptRef(new RemoteGraphicsContextGLProxyCocoa(WebProcess::singleton().ensureGPUProcessConnection(), attributes, renderingBackend));
    if (!context->isValid())
        return nullptr;
    return context;
}

}

#endif
