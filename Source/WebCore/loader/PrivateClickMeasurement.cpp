/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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

#include "config.h"
#include "PrivateClickMeasurement.h"

#include "Logging.h"
#include "RuntimeEnabledFeatures.h"
#include <wtf/CrossThreadCopier.h>
#include <wtf/Expected.h>
#include <wtf/RandomNumber.h>
#include <wtf/URL.h>
#include <wtf/text/StringConcatenateNumbers.h>
#include <wtf/text/StringToIntegerConversion.h>
#include <wtf/text/StringView.h>

namespace WebCore {

static const char privateClickMeasurementTriggerAttributionPath[] = "/.well-known/private-click-measurement/trigger-attribution/";
static const char privateClickMeasurementTokenSignaturePath[] = "/.well-known/private-click-measurement/sign-unlinkable-token/";
static const char privateClickMeasurementTokenPublicKeyPath[] = "/.well-known/private-click-measurement/get-token-public-key/";
static const char privateClickMeasurementReportAttributionPath[] = "/.well-known/private-click-measurement/report-attribution/";
const size_t privateClickMeasurementAttributionTriggerDataPathSegmentSize = 2;
const size_t privateClickMeasurementPriorityPathSegmentSize = 2;
const uint8_t privateClickMeasurementVersion = 2;

const Seconds PrivateClickMeasurement::maxAge()
{
    return 24_h * 7;
};

bool PrivateClickMeasurement::isValid() const
{
    return m_attributionTriggerData
        && m_attributionTriggerData.value().isValid()
        && !m_sourceSite.registrableDomain.isEmpty()
        && !m_destinationSite.registrableDomain.isEmpty()
        && (m_timesToSend.sourceEarliestTimeToSend || m_timesToSend.destinationEarliestTimeToSend);
}

PrivateClickMeasurement::SourceSecretToken PrivateClickMeasurement::SourceSecretToken::isolatedCopy() const
{
    return {
        tokenBase64URL.isolatedCopy(),
        signatureBase64URL.isolatedCopy(),
        keyIDBase64URL.isolatedCopy(),
    };
}

PrivateClickMeasurement::EphemeralSourceNonce PrivateClickMeasurement::EphemeralSourceNonce::isolatedCopy() const
{
    return { nonce.isolatedCopy() };
}

PrivateClickMeasurement::SourceUnlinkableToken PrivateClickMeasurement::SourceUnlinkableToken::isolatedCopy() const
{
    return {
#if PLATFORM(COCOA)
        blinder,
        waitingToken,
        readyToken,
#endif
        valueBase64URL.isolatedCopy()
    };
}

PrivateClickMeasurement PrivateClickMeasurement::isolatedCopy() const
{
    PrivateClickMeasurement copy {
        m_sourceID,
        m_sourceSite.isolatedCopy(),
        m_destinationSite.isolatedCopy(),
        m_sourceApplicationBundleID.isolatedCopy(),
        m_timeOfAdClick.isolatedCopy(),
        m_isEphemeral,
    };
    copy.m_attributionTriggerData = m_attributionTriggerData;
    copy.m_timesToSend = m_timesToSend;
    copy.m_ephemeralSourceNonce = crossThreadCopy(m_ephemeralSourceNonce);
    copy.m_sourceUnlinkableToken = m_sourceUnlinkableToken.isolatedCopy();
    copy.m_sourceSecretToken = crossThreadCopy(m_sourceSecretToken);
    return copy;
}

Expected<PrivateClickMeasurement::AttributionTriggerData, String> PrivateClickMeasurement::parseAttributionRequestQuery(const URL& redirectURL)
{
    if (!redirectURL.hasQuery())
        return AttributionTriggerData { };

    auto parameters = queryParameters(redirectURL);
    if (!parameters.size())
        return makeUnexpected("[Private Click Measurement] Conversion was not accepted because the URL had a query string but it didn't contain supported parameters."_s);

    if (parameters.size() > 1)
        return makeUnexpected("[Private Click Measurement] Conversion was not accepted because the URL's query string contained unsupported parameters."_s);

    auto parameter = parameters.first();
    if (parameter.key == "attributionSource") {
        if (parameter.value.isEmpty())
            return makeUnexpected("[Private Click Measurement] Conversion was not accepted because the URL's attributionSource query parameter had no value."_s);

        auto attributionSourceURL = URL(URL(), parameter.value);
        if (!attributionSourceURL.isValid() || (attributionSourceURL.hasPath() && attributionSourceURL.path().length() > 1) || attributionSourceURL.hasCredentials() || attributionSourceURL.hasQuery() || attributionSourceURL.hasFragmentIdentifier())
            return makeUnexpected("[Private Click Measurement] Conversion was not accepted because the URL's attributionSource query parameter was not a valid URL or was a URL with a path, credentials, query string, or fragment."_s);

        AttributionTriggerData attributionTriggerData;
        attributionTriggerData.sourceRegistrableDomain = RegistrableDomain { attributionSourceURL };
        return attributionTriggerData;
    }

    return makeUnexpected("[Private Click Measurement] Conversion was not accepted because the URL did not contain an attributionSource query parameter."_s);
}

Expected<PrivateClickMeasurement::AttributionTriggerData, String> PrivateClickMeasurement::parseAttributionRequest(const URL& redirectURL)
{
    auto path = StringView(redirectURL.string()).substring(redirectURL.pathStart(), redirectURL.pathEnd() - redirectURL.pathStart());
    if (path.isEmpty() || !path.startsWith(privateClickMeasurementTriggerAttributionPath))
        return makeUnexpected(nullString());

    if (!redirectURL.protocolIs("https") || redirectURL.hasCredentials() || redirectURL.hasFragmentIdentifier())
        return makeUnexpected("[Private Click Measurement] Conversion was not accepted because the URL's protocol is not HTTPS or the URL contains one or more of username, password, and fragment."_s);

    auto result = parseAttributionRequestQuery(redirectURL);
    if (!result && !result.error().isEmpty())
        return result;
    auto attributionTriggerData = result.value();

    auto prefixLength = sizeof(privateClickMeasurementTriggerAttributionPath) - 1;
    if (path.length() == prefixLength + privateClickMeasurementAttributionTriggerDataPathSegmentSize) {
        auto attributionTriggerDataUInt64 = parseInteger<uint64_t>(path.substring(prefixLength, privateClickMeasurementAttributionTriggerDataPathSegmentSize));
        if (!attributionTriggerDataUInt64 || *attributionTriggerDataUInt64 > AttributionTriggerData::MaxEntropy)
            return makeUnexpected(makeString("[Private Click Measurement] Conversion was not accepted because the conversion data could not be parsed or was higher than the allowed maximum of "_s, AttributionTriggerData::MaxEntropy, "."_s));

        attributionTriggerData.data = static_cast<uint8_t>(*attributionTriggerDataUInt64);
        attributionTriggerData.priority = 0;
        return attributionTriggerData;
    }
    
    if (path.length() == prefixLength + privateClickMeasurementAttributionTriggerDataPathSegmentSize + 1 + privateClickMeasurementPriorityPathSegmentSize) {
        auto attributionTriggerDataUInt64 = parseInteger<uint64_t>(path.substring(prefixLength, privateClickMeasurementAttributionTriggerDataPathSegmentSize));
        if (!attributionTriggerDataUInt64 || *attributionTriggerDataUInt64 > AttributionTriggerData::MaxEntropy)
            return makeUnexpected(makeString("[Private Click Measurement] Conversion was not accepted because the conversion data could not be parsed or was higher than the allowed maximum of "_s, AttributionTriggerData::MaxEntropy, "."_s));

        auto attributionPriorityUInt64 = parseInteger<uint64_t>(path.substring(prefixLength + privateClickMeasurementAttributionTriggerDataPathSegmentSize + 1, privateClickMeasurementPriorityPathSegmentSize));
        if (!attributionPriorityUInt64 || *attributionPriorityUInt64 > Priority::MaxEntropy)
            return makeUnexpected(makeString("[Private Click Measurement] Conversion was not accepted because the priority could not be parsed or was higher than the allowed maximum of "_s, Priority::MaxEntropy, "."_s));

        attributionTriggerData.data = static_cast<uint8_t>(*attributionTriggerDataUInt64);
        attributionTriggerData.priority = static_cast<uint8_t>(*attributionPriorityUInt64);
        return attributionTriggerData;
    }

    return makeUnexpected("[Private Click Measurement] Conversion was not accepted because the URL path contained unrecognized parts."_s);
}

bool PrivateClickMeasurement::hasPreviouslyBeenReported()
{
    return !m_timesToSend.sourceEarliestTimeToSend || !m_timesToSend.destinationEarliestTimeToSend;
}

void PrivateClickMeasurement::setSourceApplicationBundleIDForTesting(const String& appBundleIDForTesting)
{
    m_sourceApplicationBundleID = appBundleIDForTesting;
}

static Seconds randomlyBetweenTwentyFourAndFortyEightHours(PrivateClickMeasurement::IsRunningLayoutTest isRunningTest)
{
    return isRunningTest == PrivateClickMeasurement::IsRunningLayoutTest::Yes ? 1_s : 24_h + Seconds(randomNumber() * (24_h).value());
}

PrivateClickMeasurement::AttributionSecondsUntilSendData PrivateClickMeasurement::attributeAndGetEarliestTimeToSend(AttributionTriggerData&& attributionTriggerData, IsRunningLayoutTest isRunningTest)
{
    if (!attributionTriggerData.isValid() || (m_attributionTriggerData && m_attributionTriggerData->priority >= attributionTriggerData.priority))
        return { };

    m_attributionTriggerData = WTFMove(attributionTriggerData);
    // 24-48 hour delay before sending. This helps privacy since the conversion and the attribution
    // requests are detached and the time of the attribution does not reveal the time of the conversion.
    auto sourceSecondsUntilSend = randomlyBetweenTwentyFourAndFortyEightHours(isRunningTest);
    auto destinationSecondsUntilSend = randomlyBetweenTwentyFourAndFortyEightHours(isRunningTest);
    m_timesToSend = { WallTime::now() + sourceSecondsUntilSend, WallTime::now() + destinationSecondsUntilSend };

    return AttributionSecondsUntilSendData { sourceSecondsUntilSend, destinationSecondsUntilSend };
}

bool PrivateClickMeasurement::hasHigherPriorityThan(const PrivateClickMeasurement& other) const
{
    if (!other.m_attributionTriggerData)
        return true;
    
    if (!m_attributionTriggerData)
        return false;

    return m_attributionTriggerData->priority > other.m_attributionTriggerData->priority;
}

static URL makeValidURL(const RegistrableDomain& domain, const char* path)
{
    URL validURL { { }, makeString("https://", domain.string(), path) };
    return validURL.isValid() ? validURL : URL { };
}

static URL attributionReportURL(const RegistrableDomain& domain)
{
    return makeValidURL(domain, privateClickMeasurementReportAttributionPath);
}

URL PrivateClickMeasurement::attributionReportClickSourceURL() const
{
    if (!isValid())
        return URL();

    return attributionReportURL(m_sourceSite.registrableDomain);
}

URL PrivateClickMeasurement::attributionReportClickDestinationURL() const
{
    if (!isValid())
        return URL();

    return attributionReportURL(m_destinationSite.registrableDomain);
}

Ref<JSON::Object> PrivateClickMeasurement::attributionReportJSON() const
{
    auto reportDetails = JSON::Object::create();
    if (!m_attributionTriggerData || !isValid())
        return reportDetails;

    reportDetails->setString("source_engagement_type"_s, "click"_s);
    reportDetails->setString("source_site"_s, m_sourceSite.registrableDomain.string());
    reportDetails->setInteger("source_id"_s, m_sourceID.id);
    reportDetails->setString("attributed_on_site"_s, m_destinationSite.registrableDomain.string());
    reportDetails->setInteger("trigger_data"_s, m_attributionTriggerData->data);
    reportDetails->setInteger("version"_s, privateClickMeasurementVersion);

    // This token has been kept secret this far and cannot be linked to the unlinkable token.
    if (m_sourceSecretToken) {
        reportDetails->setString("source_secret_token"_s, m_sourceSecretToken->tokenBase64URL);
        reportDetails->setString("source_secret_token_signature"_s, m_sourceSecretToken->signatureBase64URL);
    }

    return reportDetails;
}

// MARK: - Fraud Prevention

static constexpr uint32_t EphemeralSourceNonceRequiredNumberOfBytes = 16;

bool PrivateClickMeasurement::EphemeralSourceNonce::isValid() const
{
    // FIXME: Investigate if we can do with a simple length check instead of decoding.
    // https://bugs.webkit.org/show_bug.cgi?id=221945
    auto digest = base64URLDecode(nonce);
    if (!digest)
        return false;
    return digest->size() == EphemeralSourceNonceRequiredNumberOfBytes;
}

void PrivateClickMeasurement::setEphemeralSourceNonce(EphemeralSourceNonce&& nonce)
{
    if (!nonce.isValid())
        return;
    m_ephemeralSourceNonce = WTFMove(nonce);
}

URL PrivateClickMeasurement::tokenSignatureURL() const
{
    if (!m_ephemeralSourceNonce || !m_ephemeralSourceNonce->isValid())
        return URL();

    return makeValidURL(m_sourceSite.registrableDomain, privateClickMeasurementTokenSignaturePath);
}

URL PrivateClickMeasurement::tokenPublicKeyURL() const
{
    return makeValidURL(m_sourceSite.registrableDomain, privateClickMeasurementTokenPublicKeyPath);
}

Ref<JSON::Object> PrivateClickMeasurement::tokenSignatureJSON() const
{
    auto reportDetails = JSON::Object::create();
    if (!m_ephemeralSourceNonce || !m_ephemeralSourceNonce->isValid())
        return reportDetails;

    if (m_sourceUnlinkableToken.valueBase64URL.isEmpty())
        return reportDetails;

    reportDetails->setString("source_engagement_type"_s, "click"_s);
    reportDetails->setString("source_nonce"_s, m_ephemeralSourceNonce->nonce);
    // This token can not be linked to the secret token.
    reportDetails->setString("source_unlinkable_token"_s, m_sourceUnlinkableToken.valueBase64URL);
    reportDetails->setInteger("version"_s, privateClickMeasurementVersion);
    return reportDetails;
}

void PrivateClickMeasurement::setSourceSecretToken(SourceSecretToken&& token)
{
    if (!token.isValid())
        return;
    m_sourceSecretToken = WTFMove(token);
}

bool PrivateClickMeasurement::SourceSecretToken::isValid() const
{
    return !(tokenBase64URL.isEmpty() || signatureBase64URL.isEmpty() || keyIDBase64URL.isEmpty());
}

} // namespace WebCore
