/*
 * Copyright (C) 2012-2021 Apple Inc. All rights reserved.
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

#if ENABLE(JIT)

#include "ExecutableAllocator.h"
#include "MacroAssemblerCodeRef.h"

namespace JSC {

class JITStubRoutineSet;
class VM;

#if USE(JSVALUE64)
using StructureID = uint32_t;
#else
using StructureID = Structure*;
#endif

class AccessCase;

// This is a base-class for JIT stub routines, and also the class you want
// to instantiate directly if you have a routine that does not need any
// help from the GC. If in doubt, use one of the other stub routines. But
// if you know for sure that the stub routine cannot be on the stack while
// someone triggers a stub routine reset, then using this will speed up
// memory reclamation. One case where a stub routine satisfies this
// condition is if it doesn't make any calls, to either C++ or JS code. In
// such a routine you know that it cannot be on the stack when anything
// interesting happens.
// See GCAwareJITStubRoutine.h for the other stub routines.
class JITStubRoutine {
    WTF_MAKE_NONCOPYABLE(JITStubRoutine);
    WTF_MAKE_FAST_ALLOCATED;
public:
    JITStubRoutine(const MacroAssemblerCodeRef<JITStubRoutinePtrTag>& code)
        : m_code(code)
        , m_refCount(1)
    {
    }
    
    virtual ~JITStubRoutine();
    virtual void aboutToDie() { }
    
    // MacroAssemblerCodeRef is copyable, but at the cost of reference
    // counting churn. Returning a reference is a good way of reducing
    // the churn.
    const MacroAssemblerCodeRef<JITStubRoutinePtrTag>& code() const { return m_code; }
    
    static MacroAssemblerCodePtr<JITStubRoutinePtrTag> asCodePtr(Ref<JITStubRoutine>&& stubRoutine)
    {
        MacroAssemblerCodePtr<JITStubRoutinePtrTag> result = stubRoutine->code().code();
        ASSERT(!!result);
        return result;
    }
    
    void ref()
    {
        m_refCount++;
    }
    
    void deref()
    {
        if (--m_refCount)
            return;
        observeZeroRefCount();
    }
    
    // Helpers for the GC to determine how to deal with marking JIT stub
    // routines.
    uintptr_t startAddress() const { return m_code.executableMemory()->startAsInteger(); }
    uintptr_t endAddress() const { return m_code.executableMemory()->endAsInteger(); }
    static uintptr_t addressStep() { return jitAllocationGranule; }
    
    static bool passesFilter(uintptr_t address)
    {
        return isJITPC(bitwise_cast<void*>(address));
    }
    
    // Return true if you are still valid after. Return false if you are now invalid. If you return
    // false, you will usually not do any clearing because the idea is that you will simply be
    // destroyed.
    virtual bool visitWeak(VM&);

protected:
    virtual void observeZeroRefCount();

    MacroAssemblerCodeRef<JITStubRoutinePtrTag> m_code;
    unsigned m_refCount;
    mutable unsigned m_hash { 0 };
};

// Helper for the creation of simple stub routines that need no help from the GC.
#define FINALIZE_CODE_FOR_STUB(codeBlock, patchBuffer, resultPtrTag, ...) \
    (adoptRef(new JITStubRoutine(FINALIZE_CODE_FOR((codeBlock), (patchBuffer), (resultPtrTag), __VA_ARGS__))))

} // namespace JSC

#endif // ENABLE(JIT)
