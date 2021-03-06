/*
 * Copyright (C) 2004, 2005, 2006, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005 Rob Buis <buis@kde.org>
 * Copyright (C) 2005 Eric Seidel <eric@webkit.org>
 * Copyright (C) 2021 Apple Inc.  All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "FilterEffect.h"
#include <wtf/Vector.h>

namespace WebCore {

enum ComponentTransferType {
    FECOMPONENTTRANSFER_TYPE_UNKNOWN  = 0,
    FECOMPONENTTRANSFER_TYPE_IDENTITY = 1,
    FECOMPONENTTRANSFER_TYPE_TABLE    = 2,
    FECOMPONENTTRANSFER_TYPE_DISCRETE = 3,
    FECOMPONENTTRANSFER_TYPE_LINEAR   = 4,
    FECOMPONENTTRANSFER_TYPE_GAMMA    = 5
};

struct ComponentTransferFunction {
    ComponentTransferFunction() = default;

    ComponentTransferType type { FECOMPONENTTRANSFER_TYPE_UNKNOWN };

    float slope { 0 };
    float intercept { 0 };
    float amplitude { 0 };
    float exponent { 0 };
    float offset { 0 };

    Vector<float> tableValues;
};

class FEComponentTransfer : public FilterEffect {
public:
    static Ref<FEComponentTransfer> create(const ComponentTransferFunction& redFunc, const ComponentTransferFunction& greenFunc, const ComponentTransferFunction& blueFunc, const ComponentTransferFunction& alphaFunc);

    ComponentTransferFunction redFunction() const { return m_redFunction; }
    ComponentTransferFunction greenFunction() const { return m_greenFunction; }
    ComponentTransferFunction blueFunction() const { return m_blueFunction; }
    ComponentTransferFunction alphaFunction() const { return m_alphaFunction; }

private:
    FEComponentTransfer(const ComponentTransferFunction& redFunc, const ComponentTransferFunction& greenFunc, const ComponentTransferFunction& blueFunc, const ComponentTransferFunction& alphaFunc);

#if USE(CORE_IMAGE)
    bool supportsCoreImageRendering() const override;
#endif

    std::unique_ptr<FilterEffectApplier> createApplier(const Filter&) const override;

    WTF::TextStream& externalRepresentation(WTF::TextStream&, RepresentationType) const override;

    ComponentTransferFunction m_redFunction;
    ComponentTransferFunction m_greenFunction;
    ComponentTransferFunction m_blueFunction;
    ComponentTransferFunction m_alphaFunction;
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_FILTER_EFFECT(FEComponentTransfer)
