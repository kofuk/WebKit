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
#import "ScrollingAccelerationCurve.h"

#if ENABLE(MOMENTUM_EVENT_DISPATCHER) && PLATFORM(MAC)

#import "Logging.h"
#import <pal/spi/cg/CoreGraphicsSPI.h>
#import <pal/spi/cocoa/IOKitSPI.h>

namespace WebKit {

static float fromFixedPoint(float value)
{
    return value / 65536.0f;
}

static float readFixedPointParameter(NSDictionary *parameters, const char *key)
{
    return fromFixedPoint([[parameters objectForKey:@(key)] floatValue]);
}

static ScrollingAccelerationCurve fromIOHIDCurve(NSDictionary *parameters, float resolution)
{
    auto gainLinear = readFixedPointParameter(parameters, kHIDAccelGainLinearKey);
    auto gainParabolic = readFixedPointParameter(parameters, kHIDAccelGainParabolicKey);
    auto gainCubic = readFixedPointParameter(parameters, kHIDAccelGainCubicKey);
    auto gainQuartic = readFixedPointParameter(parameters, kHIDAccelGainQuarticKey);

    auto tangentSpeedLinear = readFixedPointParameter(parameters, kHIDAccelTangentSpeedLinearKey);
    auto tangentSpeedParabolicRoot = readFixedPointParameter(parameters, kHIDAccelTangentSpeedParabolicRootKey);

    return { gainLinear, gainParabolic, gainCubic, gainQuartic, tangentSpeedLinear, tangentSpeedParabolicRoot, resolution };
}

static ScrollingAccelerationCurve fromIOHIDCurveArrayWithAcceleration(NSArray<NSDictionary *> *ioHIDCurves, float desiredAcceleration, float resolution)
{
    __block size_t currentIndex = 0;
    __block Vector<std::pair<float, ScrollingAccelerationCurve>> curves;

    [ioHIDCurves enumerateObjectsUsingBlock:^(NSDictionary *parameters, NSUInteger i, BOOL *) {
        auto curveAcceleration = readFixedPointParameter(parameters, kHIDAccelIndexKey);
        auto curve = fromIOHIDCurve(parameters, resolution);

        if (desiredAcceleration > curveAcceleration)
            currentIndex = i;

        curves.append({ curveAcceleration, curve });
    }];

    // Interpolation if desiredAcceleration is in between two curves.
    if (curves[currentIndex].first < desiredAcceleration && (currentIndex + 1) < curves.size()) {
        const auto& lowCurve = curves[currentIndex];
        const auto& highCurve = curves[currentIndex + 1];
        float ratio = (desiredAcceleration - lowCurve.first) / (highCurve.first - lowCurve.first);
        return ScrollingAccelerationCurve::interpolate(lowCurve.second, highCurve.second, ratio);
    }

    return curves[currentIndex].second;
}

static RetainPtr<IOHIDEventSystemClientRef> createHIDClient()
{
    auto client = adoptCF(IOHIDEventSystemClientCreateWithType(nil, kIOHIDEventSystemClientTypePassive, nil));
    IOHIDEventSystemClientSetDispatchQueue(client.get(), dispatch_get_main_queue());
    IOHIDEventSystemClientActivate(client.get());
    return client;
}

static std::optional<ScrollingAccelerationCurve> fromIOHIDDevice(IOHIDEventSenderID senderID)
{
    static NeverDestroyed<RetainPtr<IOHIDEventSystemClientRef>> client;
    if (!client.get())
        client.get() = createHIDClient();

    RetainPtr<IOHIDServiceClientRef> ioHIDService = adoptCF(IOHIDEventSystemClientCopyServiceForRegistryID(client.get().get(), senderID));
    if (!ioHIDService) {
        RELEASE_LOG(ScrollAnimations, "ScrollingAccelerationCurve::fromIOHIDDevice did not find matching HID service");
        return std::nullopt;
    }

    auto curves = adoptCF(dynamic_cf_cast<CFArrayRef>(IOHIDServiceClientCopyProperty(ioHIDService.get(), CFSTR(kHIDScrollAccelParametricCurvesKey))));
    if (!curves) {
        RELEASE_LOG(ScrollAnimations, "ScrollingAccelerationCurve::fromIOHIDDevice failed to look up curves");
        return std::nullopt;
    }

    // FIXME: There is some additional fallback to implement here, though this seems usually sufficient.
    auto scrollAccelerationType = adoptCF(dynamic_cf_cast<CFStringRef>(IOHIDServiceClientCopyProperty(ioHIDService.get(), CFSTR("HIDScrollAccelerationType"))));
    if (!scrollAccelerationType) {
        RELEASE_LOG(ScrollAnimations, "ScrollingAccelerationCurve::fromIOHIDDevice failed to look up acceleration type");
        return std::nullopt;
    }

    auto scrollAcceleration = adoptCF(dynamic_cf_cast<CFNumberRef>(IOHIDServiceClientCopyProperty(ioHIDService.get(), scrollAccelerationType.get())));
    if (!scrollAcceleration) {
        RELEASE_LOG(ScrollAnimations, "ScrollingAccelerationCurve::fromIOHIDDevice failed to look up acceleration value");
        return std::nullopt;
    }

    auto resolution = adoptCF(dynamic_cf_cast<CFNumberRef>(IOHIDServiceClientCopyProperty(ioHIDService.get(), CFSTR(kIOHIDScrollResolutionKey))));
    if (!resolution) {
        RELEASE_LOG(ScrollAnimations, "ScrollingAccelerationCurve::fromIOHIDDevice failed to look up resolution");
        return std::nullopt;
    }

    return fromIOHIDCurveArrayWithAcceleration((NSArray *)curves.get(), fromFixedPoint([(NSNumber *)scrollAcceleration.get() floatValue]), fromFixedPoint([(NSNumber *)resolution.get() floatValue]));
}

std::optional<ScrollingAccelerationCurve> ScrollingAccelerationCurve::fromNativeWheelEvent(const NativeWebWheelEvent& nativeWebWheelEvent)
{
    NSEvent *event = nativeWebWheelEvent.nativeEvent();

    auto cgEvent = event.CGEvent;
    if (!cgEvent) {
        RELEASE_LOG(ScrollAnimations, "ScrollingAccelerationCurve::fromNativeWheelEvent did not find CG event");
        return std::nullopt;
    }

    auto hidEvent = adoptCF(CGEventCopyIOHIDEvent(cgEvent));
    if (!hidEvent) {
        RELEASE_LOG(ScrollAnimations, "ScrollingAccelerationCurve::fromNativeWheelEvent did not find HID event");
        return std::nullopt;
    }

    return fromIOHIDDevice(IOHIDEventGetSenderID(hidEvent.get()));
}

} // namespace WebKit

#endif // ENABLE(MOMENTUM_EVENT_DISPATCHER) && PLATFORM(MAC)
