/*
 * Copyright (C) 2011-2021 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#pragma once

#include "Filter.h"
#include "IntRectExtent.h"
#include "LayoutRect.h"
#include <wtf/TypeCasts.h>

namespace WebCore {

class FilterEffect;
class FilterOperations;
class GraphicsContext;
class ReferenceFilterOperation;
class RenderElement;
class SourceGraphic;

class CSSFilter final : public Filter {
    WTF_MAKE_FAST_ALLOCATED;
public:
    static RefPtr<CSSFilter> create(const FilterOperations&, RenderingMode, float scaleFactor = 1, ClipOperation = ClipOperation::Intersect);

    void setSourceImageRect(const FloatRect&);
    bool buildFilterFunctions(RenderElement&, const FilterOperations&);
    void determineFilterPrimitiveSubregion();

    bool hasFilterThatMovesPixels() const { return m_hasFilterThatMovesPixels; }
    bool hasFilterThatShouldBeRestrictedBySecurityOrigin() const { return m_hasFilterThatShouldBeRestrictedBySecurityOrigin; }

    RefPtr<FilterEffect> lastEffect();
    IntOutsets outsets() const override;

    void clearIntermediateResults();
    RefPtr<FilterImage> apply() override;

    bool updateBackingStoreRect(const FloatRect& filterRect);

    LayoutRect computeSourceImageRectForDirtyRect(const LayoutRect& filterBoxRect, const LayoutRect& dirtyRect);

private:
    CSSFilter(RenderingMode, float scaleFactor, ClipOperation, bool hasFilterThatMovesPixels, bool hasFilterThatShouldBeRestrictedBySecurityOrigin);

#if USE(CORE_IMAGE)
    bool supportsCoreImageRendering() const override;
#endif

    bool m_hasFilterThatMovesPixels { false };
    bool m_hasFilterThatShouldBeRestrictedBySecurityOrigin { false };

    Vector<Ref<FilterFunction>> m_functions;

    mutable IntOutsets m_outsets;
};

} // namespace WebCore

SPECIALIZE_TYPE_TRAITS_BEGIN(WebCore::CSSFilter)
    static bool isType(const WebCore::Filter& filter) { return filter.isCSSFilter(); }
SPECIALIZE_TYPE_TRAITS_END()
