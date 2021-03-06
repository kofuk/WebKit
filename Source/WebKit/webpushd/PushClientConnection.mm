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
#import "PushClientConnection.h"

#import "AppBundleRequest.h"
#import "CodeSigning.h"
#import "WebPushDaemon.h"
#import "WebPushDaemonConnectionConfiguration.h"
#import <JavaScriptCore/ConsoleTypes.h>
#import <wtf/Vector.h>
#import <wtf/cocoa/Entitlements.h>

namespace WebPushD {

Ref<ClientConnection> ClientConnection::create(xpc_connection_t connection)
{
    return adoptRef(*new ClientConnection(connection));
}

ClientConnection::ClientConnection(xpc_connection_t connection)
    : m_xpcConnection(connection)
{
}

void ClientConnection::updateConnectionConfiguration(const WebPushDaemonConnectionConfiguration& configuration)
{
    if (configuration.hostAppAuditTokenData)
        setHostAppAuditTokenData(*configuration.hostAppAuditTokenData);

    m_useMockBundlesForTesting = configuration.useMockBundlesForTesting;
}

void ClientConnection::setHostAppAuditTokenData(const Vector<uint8_t>& tokenData)
{
    audit_token_t token;
    if (tokenData.size() != sizeof(token)) {
        ASSERT_WITH_MESSAGE(false, "Attempt to set an audit token from incorrect number of bytes");
        return;
    }

    memcpy(&token, tokenData.data(), tokenData.size());

    if (hasHostAppAuditToken()) {
        // Verify the token being set is equivalent to the last one set
        audit_token_t& existingAuditToken = *m_hostAppAuditToken;
        RELEASE_ASSERT(!memcmp(&existingAuditToken, &token, sizeof(token)));
        return;
    }

    m_hostAppAuditToken = WTFMove(token);
}

const String& ClientConnection::hostAppCodeSigningIdentifier()
{
    if (!m_hostAppCodeSigningIdentifier) {
        if (!m_hostAppAuditToken)
            m_hostAppCodeSigningIdentifier = String();
        else
            m_hostAppCodeSigningIdentifier = WebKit::codeSigningIdentifier(*m_hostAppAuditToken);
    }

    return *m_hostAppCodeSigningIdentifier;
}

bool ClientConnection::hostAppHasPushEntitlement()
{
    if (!m_hostAppHasPushEntitlement) {
        if (!m_hostAppAuditToken)
            return false;
        m_hostAppHasPushEntitlement = WTF::hasEntitlement(*m_hostAppAuditToken, "com.apple.private.webkit.webpush");
    }

    return *m_hostAppHasPushEntitlement;
}

void ClientConnection::setDebugModeIsEnabled(bool enabled)
{
    if (enabled == m_debugModeEnabled)
        return;

    m_debugModeEnabled = enabled;

    auto identifier = hostAppCodeSigningIdentifier();
    String message;
    if (!identifier.isEmpty())
        message = makeString("[webpushd - ", identifier, "] Turned Debug Mode ", m_debugModeEnabled ? "on" : "off");
    else
        message = makeString("[webpushd] Turned Debug Mode ", m_debugModeEnabled ? "on" : "off");

    Daemon::singleton().broadcastDebugMessage(MessageLevel::Info, message);
}

void ClientConnection::enqueueAppBundleRequest(std::unique_ptr<AppBundleRequest>&& request)
{
    RELEASE_ASSERT(m_xpcConnection);
    m_pendingBundleRequests.append(WTFMove(request));
    maybeStartNextAppBundleRequest();
}

void ClientConnection::maybeStartNextAppBundleRequest()
{
    RELEASE_ASSERT(m_xpcConnection);

    if (m_currentBundleRequest || m_pendingBundleRequests.isEmpty())
        return;

    m_currentBundleRequest = m_pendingBundleRequests.takeFirst();
    m_currentBundleRequest->start();
}

void ClientConnection::didCompleteAppBundleRequest(AppBundleRequest& request)
{
    // If our connection was closed there should be no in-progress bundle requests.
    RELEASE_ASSERT(m_xpcConnection);

    ASSERT(m_currentBundleRequest.get() == &request);
    m_currentBundleRequest = nullptr;

    maybeStartNextAppBundleRequest();
}

void ClientConnection::connectionClosed()
{
    RELEASE_ASSERT(m_xpcConnection);
    m_xpcConnection = nullptr;

    if (m_currentBundleRequest) {
        m_currentBundleRequest->cancel();
        m_currentBundleRequest = nullptr;
    }

    Deque<std::unique_ptr<AppBundleRequest>> pendingBundleRequests;
    pendingBundleRequests.swap(m_pendingBundleRequests);
    for (auto& requst : pendingBundleRequests)
        requst->cancel();
}

} // namespace WebPushD
