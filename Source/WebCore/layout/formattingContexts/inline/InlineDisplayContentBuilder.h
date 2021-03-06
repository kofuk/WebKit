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

#pragma once

#if ENABLE(LAYOUT_FORMATTING_CONTEXT)

#include "InlineLineBuilder.h"
#include "LayoutUnits.h"

namespace WebCore {
namespace Layout {

class ContainerBox;
class InlineFormattingState;
class LineBox;

class InlineDisplayContentBuilder {
public:
    InlineDisplayContentBuilder(const ContainerBox& formattingContextRoot, InlineFormattingState&);

    DisplayBoxes build(const LineBuilder::LineContent&, const LineBox&, const InlineRect& lineBoxLogicalRect, const size_t lineIndex);

private:
    void processNonBidiContent(const LineBuilder::LineContent&, const LineBox&, const InlineLayoutPoint& lineBoxLogicalTopLeft, DisplayBoxes&);
    void processBidiContent(const LineBuilder::LineContent&, const LineBox&, const InlineLayoutPoint& lineBoxLogicalTopLeft, DisplayBoxes&);
    void processOverflownRunsForEllipsis(DisplayBoxes&, InlineLayoutUnit lineBoxLogicalRight);
    void collectInkOverflowForInlineBoxes(const LineBox&, DisplayBoxes&);

    void appendTextDisplayBox(const Line::Run&, const InlineRect&, DisplayBoxes&);
    void appendSoftLineBreakDisplayBox(const Line::Run&, const InlineRect&, DisplayBoxes&);
    void appendHardLineBreakDisplayBox(const Line::Run&, const InlineRect&, DisplayBoxes&);
    void appendAtomicInlineLevelDisplayBox(const Line::Run&, const InlineRect& , DisplayBoxes&);
    void appendInlineBoxDisplayBox(const Line::Run&, const InlineLevelBox&, const InlineRect&, bool linehasContent, DisplayBoxes&);
    void appendSpanningInlineBoxDisplayBox(const Line::Run&, const InlineLevelBox&, const InlineRect&, DisplayBoxes&);
    void appendInlineBoxDisplayBoxForBidiBoundary(const Box&, const InlineRect&, DisplayBoxes&);
    void adjustInlineBoxDisplayBoxForBidiBoundary(InlineDisplay::Box&, const InlineRect&);

    const ContainerBox& root() const { return m_formattingContextRoot; }
    InlineFormattingState& formattingState() const { return m_formattingState; } 

    const ContainerBox& m_formattingContextRoot;
    InlineFormattingState& m_formattingState;
    HashMap<const Box*, size_t> m_inlineBoxIndexMap;
    size_t m_lineIndex { 0 };
};

}
}

#endif
