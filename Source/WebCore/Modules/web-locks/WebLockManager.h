/*
 * Copyright (C) 2021, Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1.  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "AbortSignal.h"
#include "ActiveDOMObject.h"
#include "ClientOrigin.h"
#include "WebLockIdentifier.h"
#include "WebLockMode.h"
#include <wtf/RefCounted.h>
#include <wtf/WeakPtr.h>

namespace WebCore {

class DeferredPromise;
class NavigatorBase;
class WebLockGrantedCallback;

struct ClientOrigin;
struct WebLockManagerSnapshot;

class WebLockManager : public RefCounted<WebLockManager>, public CanMakeWeakPtr<WebLockManager>, public ActiveDOMObject {
public:
    static Ref<WebLockManager> create(NavigatorBase&);
    ~WebLockManager();

    struct Options {
        WebLockMode mode { WebLockMode::Exclusive };
        bool ifAvailable { false };
        bool steal { false };
        RefPtr<AbortSignal> signal;
    };

    using Snapshot = WebLockManagerSnapshot;

    void request(const String& name, Ref<WebLockGrantedCallback>&&, Ref<DeferredPromise>&&);
    void request(const String& name, Options&&, Ref<WebLockGrantedCallback>&&, Ref<DeferredPromise>&&);
    void query(Ref<DeferredPromise>&&);

private:
    explicit WebLockManager(NavigatorBase&);

    void requestLockOnMainThread(WebLockIdentifier, const String& name, const Options&, Function<void(bool)>&&, Function<void(Exception&&)>&& releaseHandler);
    void releaseLockOnMainThread(WebLockIdentifier, const String& name);
    void abortLockRequestOnMainThread(WebLockIdentifier, const String& name, CompletionHandler<void(bool)>&&);
    void queryOnMainThread(CompletionHandler<void(Snapshot&&)>&&);

    void didCompleteLockRequest(WebLockIdentifier, bool success);
    void settleReleasePromise(WebLockIdentifier, ExceptionOr<JSC::JSValue>&&);
    void signalToAbortTheRequest(WebLockIdentifier);
    void clientIsGoingAway();

    // ActiveDOMObject.
    void stop() final;
    const char* activeDOMObjectName() const final;
    bool virtualHasPendingActivity() const final;

    const std::optional<ClientOrigin> m_clientOrigin;
    HashMap<WebLockIdentifier, RefPtr<DeferredPromise>> m_releasePromises;

    struct LockRequest;
    HashMap<WebLockIdentifier, LockRequest> m_pendingRequests;
};

} // namespace WebCore
