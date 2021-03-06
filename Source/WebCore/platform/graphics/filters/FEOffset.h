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

namespace WebCore {

class FEOffset : public FilterEffect {
public:
    static Ref<FEOffset> create(float dx, float dy);

    float dx() const { return m_dx; }
    void setDx(float);

    float dy() const { return m_dy; }
    void setDy(float);

private:
    FEOffset(float dx, float dy);

    FloatRect calculateImageRect(const Filter&, const FilterImageVector& inputs, const FloatRect& primitiveSubregion) const override;

    bool resultIsAlphaImage(const FilterImageVector& inputs) const override;

    std::unique_ptr<FilterEffectApplier> createApplier(const Filter&) const override;

    WTF::TextStream& externalRepresentation(WTF::TextStream&, RepresentationType) const override;

    float m_dx;
    float m_dy;
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_FILTER_EFFECT(FEOffset)
