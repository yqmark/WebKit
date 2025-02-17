/*
 * Copyright (C) 2018 Yusuke Suzuki <utatane.tea@gmail.com>.
 * Copyright (C) 2018-2021 Apple Inc. All rights reserved.
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

#include "config.h"
#include "InByVariant.h"

#include "CacheableIdentifierInlines.h"

namespace JSC {

InByVariant::InByVariant(CacheableIdentifier identifier, const StructureSet& structureSet, PropertyOffset offset, const ObjectPropertyConditionSet& conditionSet)
    : m_structureSet(structureSet)
    , m_conditionSet(conditionSet)
    , m_offset(offset)
    , m_identifier(WTFMove(identifier))
{
    if (!structureSet.size()) {
        ASSERT(offset == invalidOffset);
        ASSERT(conditionSet.isEmpty());
    }
}

bool InByVariant::attemptToMerge(const InByVariant& other)
{
    if (!!m_identifier != !!other.m_identifier)
        return false;

    if (m_identifier && (m_identifier != other.m_identifier))
        return false;

    if (m_offset != other.m_offset)
        return false;

    if (m_conditionSet.isEmpty() != other.m_conditionSet.isEmpty())
        return false;

    ObjectPropertyConditionSet mergedConditionSet;
    if (!m_conditionSet.isEmpty()) {
        mergedConditionSet = m_conditionSet.mergedWith(other.m_conditionSet);
        if (!mergedConditionSet.isValid())
            return false;
        // If this is a hit variant, one slot base should exist. If this is not a hit variant, the slot base is not necessary.
        if (isHit() && !mergedConditionSet.hasOneSlotBaseCondition())
            return false;
    }
    m_conditionSet = mergedConditionSet;

    m_structureSet.merge(other.m_structureSet);

    return true;
}

template<typename Visitor>
void InByVariant::markIfCheap(Visitor& visitor)
{
    m_structureSet.markIfCheap(visitor);
}

template void InByVariant::markIfCheap(AbstractSlotVisitor&);
template void InByVariant::markIfCheap(SlotVisitor&);

bool InByVariant::finalize(VM& vm)
{
    if (!m_structureSet.isStillAlive(vm))
        return false;
    if (!m_conditionSet.areStillLive(vm))
        return false;
    return true;
}

void InByVariant::dump(PrintStream& out) const
{
    dumpInContext(out, nullptr);
}

void InByVariant::dumpInContext(PrintStream& out, DumpContext* context) const
{
    out.print("<id='", m_identifier, "', ");
    if (!isSet()) {
        out.print("empty>");
        return;
    }

    out.print(inContext(structureSet(), context), ", ", inContext(m_conditionSet, context));
    out.print(", offset = ", offset());
    out.print(">");
}

} // namespace JSC

