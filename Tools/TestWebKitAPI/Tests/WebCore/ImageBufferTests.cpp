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

#include "config.h"

#include <WebCore/ImageBuffer.h>
#include <WebCore/PlatformImageBuffer.h>

namespace TestWebKitAPI {
using namespace WebCore;

// Tests that the specialized image buffer constructors construct the expected type of object.
// Test passes if the test compiles, there was a bug where the code wouldn't compile.
TEST(ImageBufferTests, ImageBufferSubTypeCreateCreatesSubtypes)
{
    auto colorSpace = DestinationColorSpace::SRGB();
    auto pixelFormat = PixelFormat::BGRA8;
    FloatSize size { 1.f, 1.f };
    float scale = 1.f;
    RefPtr<UnacceleratedImageBuffer> unaccelerated = UnacceleratedImageBuffer::create(size, scale, colorSpace, pixelFormat, nullptr);
    RefPtr<AcceleratedImageBuffer> accelerated = AcceleratedImageBuffer::create(size, scale, colorSpace, pixelFormat, nullptr);
    RefPtr<DisplayListAcceleratedImageBuffer> displayListAccelerated = DisplayListAcceleratedImageBuffer::create(size, scale, colorSpace, pixelFormat, nullptr);
    RefPtr<DisplayListUnacceleratedImageBuffer> displayListUnaccelerated = DisplayListUnacceleratedImageBuffer::create(size, scale, colorSpace, pixelFormat, nullptr);
    EXPECT_NE(nullptr, accelerated);
    EXPECT_NE(nullptr, unaccelerated);
    EXPECT_NE(nullptr, displayListAccelerated);
    EXPECT_NE(nullptr, displayListUnaccelerated);
}

}
