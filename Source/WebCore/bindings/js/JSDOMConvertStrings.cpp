/*
 *  Copyright (C) 1999-2001 Harri Porten (porten@kde.org)
 *  Copyright (C) 2004-2011, 2013, 2016 Apple Inc. All rights reserved.
 *  Copyright (C) 2007 Samuel Weinig <sam@webkit.org>
 *  Copyright (C) 2013 Michael Pruett <michael@68k.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "config.h"
#include "JSDOMConvertStrings.h"

#include "JSDOMExceptionHandling.h"
#include <JavaScriptCore/HeapInlines.h>
#include <JavaScriptCore/JSCJSValueInlines.h>
#include <wtf/text/StringBuilder.h>
#include <wtf/unicode/CharacterNames.h>


namespace WebCore {
using namespace JSC;

String identifierToString(JSGlobalObject& lexicalGlobalObject, const Identifier& identifier)
{
    if (UNLIKELY(identifier.isSymbol())) {
        auto scope = DECLARE_THROW_SCOPE(lexicalGlobalObject.vm());
        throwTypeError(&lexicalGlobalObject, scope, SymbolCoercionError);
        return { };
    }

    return identifier.string();
}

static inline String stringToByteString(JSGlobalObject& lexicalGlobalObject, JSC::ThrowScope& scope, String&& string)
{
    if (!string.isAllLatin1()) {
        throwTypeError(&lexicalGlobalObject, scope);
        return { };
    }

    return WTFMove(string);
}

String identifierToByteString(JSGlobalObject& lexicalGlobalObject, const Identifier& identifier)
{
    VM& vm = lexicalGlobalObject.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    auto string = identifierToString(lexicalGlobalObject, identifier);
    RETURN_IF_EXCEPTION(scope, { });
    return stringToByteString(lexicalGlobalObject, scope, WTFMove(string));
}

String valueToByteString(JSGlobalObject& lexicalGlobalObject, JSValue value)
{
    VM& vm = lexicalGlobalObject.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    auto string = value.toWTFString(&lexicalGlobalObject);
    RETURN_IF_EXCEPTION(scope, { });

    return stringToByteString(lexicalGlobalObject, scope, WTFMove(string));
}

String stringToUSVString(String&& string)
{
    // Fast path for the case where there are no unpaired surrogates.
    if (!hasUnpairedSurrogate(string))
        return WTFMove(string);

    // Slow path: http://heycam.github.io/webidl/#dfn-obtain-unicode
    // Replaces unpaired surrogates with the replacement character.
    StringBuilder result;
    result.reserveCapacity(string.length());
    StringView view { string };
    for (auto codePoint : view.codePoints()) {
        if (U_IS_SURROGATE(codePoint))
            result.append(replacementCharacter);
        else
            result.appendCharacter(codePoint);
    }
    return result.toString();
}

String identifierToUSVString(JSGlobalObject& lexicalGlobalObject, const Identifier& identifier)
{
    return stringToUSVString(identifierToString(lexicalGlobalObject, identifier));
}

String valueToUSVString(JSGlobalObject& lexicalGlobalObject, JSValue value)
{
    VM& vm = lexicalGlobalObject.vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    auto string = value.toWTFString(&lexicalGlobalObject);
    RETURN_IF_EXCEPTION(scope, { });

    return stringToUSVString(WTFMove(string));
}

} // namespace WebCore
