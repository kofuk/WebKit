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

#include "FEConvolveMatrix.h"
#include "FilterEffect.h"

namespace WebCore {

class FEGaussianBlur : public FilterEffect {
public:
    static Ref<FEGaussianBlur> create(float x, float y, EdgeModeType);

    float stdDeviationX() const { return m_stdX; }
    void setStdDeviationX(float);

    float stdDeviationY() const { return m_stdY; }
    void setStdDeviationY(float);

    EdgeModeType edgeMode() const { return m_edgeMode; }
    void setEdgeMode(EdgeModeType);

    static IntSize calculateKernelSize(const Filter&, FloatSize stdDeviation);
    static IntSize calculateUnscaledKernelSize(FloatSize stdDeviation);
    static IntSize calculateOutsetSize(FloatSize stdDeviation);

private:
    FEGaussianBlur(float x, float y, EdgeModeType);

    FloatRect calculateImageRect(const Filter&, const FilterImageVector& inputs, const FloatRect& primitiveSubregion) const override;

    IntOutsets outsets() const override;

    bool resultIsAlphaImage(const FilterImageVector& inputs) const override;

    std::unique_ptr<FilterEffectApplier> createApplier(const Filter&) const override;

    WTF::TextStream& externalRepresentation(WTF::TextStream&, RepresentationType) const override;

    float m_stdX;
    float m_stdY;
    EdgeModeType m_edgeMode;
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_FILTER_EFFECT(FEGaussianBlur)
