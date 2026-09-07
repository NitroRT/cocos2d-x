/****************************************************************************
 Copyright (c) 2018-2019 Xiamen Yaji Software Co., Ltd.

 http://www.cocos2d-x.org

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
 

#include "DeviceInfoMTL.h"
#include "Utils.h"
#include "base/ccMacros.h"

#include <string>

CC_BACKEND_BEGIN

namespace {
    // Implementation limits, carried over from the MTLFeatureSet tables this file used to
    // walk and re-keyed on GPU family. MTLGPUFamily has no version axis, so each threshold
    // below sits where the old table's newest version of that family put it.

    // What Utils::getAppleGPUFamily() reports for a GPU outside the Apple series.
    constexpr int NOT_AN_APPLE_GPU = 0;

    // Maximum width and height of a 2D texture.
    constexpr int MAX_TEXTURE_DIMENSION_SMALL = 8192;    // Apple1, Apple2
    constexpr int MAX_TEXTURE_DIMENSION_LARGE = 16384;   // Apple3 and up, and Mac2
    constexpr int FIRST_LARGE_TEXTURE_APPLE_FAMILY = 3;

    // Entries in the texture argument table, per graphics or compute function. Apple splits
    // this by family too, but Metal has no runtime query to check such a split against, so
    // it stays split by OS the way the MTLFeatureSet tables had it. EMBEDDED covers every
    // Apple OS other than macOS - iOS, iPadOS, tvOS, visionOS - which is what
    // CC_PLATFORM_IOS actually selects, since CCPlatformConfig.h keys it on
    // TARGET_OS_IPHONE.
    constexpr int MAX_TEXTURE_ARGUMENT_ENTRIES_MACOS = 128;
    constexpr int MAX_TEXTURE_ARGUMENT_ENTRIES_EMBEDDED = 31;

    // Vertex attributes per vertex descriptor, and entries in the sampler state argument
    // table. Both hold for every family, so neither is worth keying on anything.
    constexpr int MAX_VERTEX_ATTRIBUTES = 31;
    constexpr int MAX_SAMPLER_ARGUMENT_ENTRIES = 16;

    int getMaxTextureWidthHeight(int appleFamily, bool isMac2)
    {
        if (appleFamily != NOT_AN_APPLE_GPU)
        {
            return appleFamily >= FIRST_LARGE_TEXTURE_APPLE_FAMILY ? MAX_TEXTURE_DIMENSION_LARGE
                                                                  : MAX_TEXTURE_DIMENSION_SMALL;
        }
        if (isMac2)
            return MAX_TEXTURE_DIMENSION_LARGE;

        // A device in neither family is one these tables predate. Report the smallest limit
        // they hold rather than 0, which would collapse texture clamping in CCTexture2D.
        return MAX_TEXTURE_DIMENSION_SMALL;
    }

    int getMaxTextureEntries()
    {
#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
        return MAX_TEXTURE_ARGUMENT_ENTRIES_MACOS;
#else
        return MAX_TEXTURE_ARGUMENT_ENTRIES_EMBEDDED;
#endif
    }
}

DeviceInfoMTL::DeviceInfoMTL(id<MTLDevice> device)
{
    _deviceName = [device.name UTF8String];
    _appleFamily = Utils::getAppleGPUFamily();

    // An Apple Silicon Mac answers to both, and the Apple tier is the more specific of the
    // two, so Mac2 is only consulted once the Apple series is ruled out.
    _isMac2 = _appleFamily == NOT_AN_APPLE_GPU && [device supportsFamily:MTLGPUFamilyMac2];

    if (_appleFamily != NOT_AN_APPLE_GPU)
        _gpuFamilyName = "Apple" + std::to_string(_appleFamily);
    else
        _gpuFamilyName = _isMac2 ? "Mac2" : "Unknown";

#if (CC_TARGET_PLATFORM == CC_PLATFORM_MAC)
    _isDepth24Stencil8PixelFormatSupported = [device isDepth24Stencil8PixelFormatSupported];
#endif
}

bool DeviceInfoMTL::init()
{
    _maxAttributes = MAX_VERTEX_ATTRIBUTES;
    _maxSamplesAllowed = MAX_SAMPLER_ARGUMENT_ENTRIES;
    _maxTextureUnits = getMaxTextureEntries();
    _maxTextureSize = getMaxTextureWidthHeight(_appleFamily, _isMac2);

    return true;
}

const char* DeviceInfoMTL::getVendor() const
{
    return "";
}

const char* DeviceInfoMTL::getRenderer() const
{
    return _deviceName.c_str();
}

const char* DeviceInfoMTL::getVersion() const
{
    return _gpuFamilyName.c_str();
}

const char* DeviceInfoMTL::getExtension() const
{
    return "";
}

bool DeviceInfoMTL::checkForFeatureSupported(FeatureType feature)
{
    // For a compressed format the honest answer is whether this backend can hand Metal a
    // pixel format for it on this device, which is exactly what the texture path asks.
    // Anything else would let the capability and the texture creation disagree.
    auto supportsFormat = [](PixelFormat format) {
        return Utils::toMTLPixelFormat(format) != MTLPixelFormatInvalid;
    };

    switch (feature)
    {
    case FeatureType::PVRTC:
        return supportsFormat(PixelFormat::PVRTC4);
    case FeatureType::ETC1:
        return supportsFormat(PixelFormat::ETC);
    case FeatureType::ETC2:
        return supportsFormat(PixelFormat::ETC2_RGBA);
    case FeatureType::S3TC:
        return supportsFormat(PixelFormat::S3TC_DXT1);
    case FeatureType::ASTC:
        // The backend has no ASTC mapping, so claiming hardware support would be a lie.
        return false;
    case FeatureType::IMG_FORMAT_BGRA8888:
        return true;
    case FeatureType::PACKED_DEPTH_STENCIL:
        return _isDepth24Stencil8PixelFormatSupported;
    default:
        return false;
    }
}

CC_BACKEND_END
