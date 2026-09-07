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
 
#include "DeviceInfoGL.h"
#include "platform/CCGL.h"
#include "base/ccMacros.h"
#include "base/CCConsole.h"

#if defined(COCOS2D_DEBUG) && COCOS2D_DEBUG > 0
#define CC_GL_DEBUG_OUTPUT_SUPPORTED 1
#else
#define CC_GL_DEBUG_OUTPUT_SUPPORTED 0
#endif

#if CC_GL_DEBUG_OUTPUT_SUPPORTED
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
#include <EGL/egl.h>

// The GLES headers carry only the _KHR spelling, with the values GL 4.3 later made core, so
// both backends are aliased to one set of names and the code below reads the same on either.
#define CC_GL_DEBUG_APIENTRY                GL_APIENTRY
#define CC_GL_DEBUG_OUTPUT                  GL_DEBUG_OUTPUT_KHR
#define CC_GL_DEBUG_OUTPUT_SYNCHRONOUS      GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR
#define CC_GL_DEBUG_SOURCE_API              GL_DEBUG_SOURCE_API_KHR
#define CC_GL_DEBUG_SOURCE_WINDOW_SYSTEM    GL_DEBUG_SOURCE_WINDOW_SYSTEM_KHR
#define CC_GL_DEBUG_SOURCE_SHADER_COMPILER  GL_DEBUG_SOURCE_SHADER_COMPILER_KHR
#define CC_GL_DEBUG_SOURCE_THIRD_PARTY      GL_DEBUG_SOURCE_THIRD_PARTY_KHR
#define CC_GL_DEBUG_SOURCE_APPLICATION      GL_DEBUG_SOURCE_APPLICATION_KHR
#define CC_GL_DEBUG_TYPE_ERROR              GL_DEBUG_TYPE_ERROR_KHR
#define CC_GL_DEBUG_TYPE_DEPRECATED         GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_KHR
#define CC_GL_DEBUG_TYPE_UNDEFINED          GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_KHR
#define CC_GL_DEBUG_TYPE_PORTABILITY        GL_DEBUG_TYPE_PORTABILITY_KHR
#define CC_GL_DEBUG_TYPE_PERFORMANCE        GL_DEBUG_TYPE_PERFORMANCE_KHR
#define CC_GL_DEBUG_TYPE_MARKER             GL_DEBUG_TYPE_MARKER_KHR
#define CC_GL_DEBUG_SEVERITY_HIGH           GL_DEBUG_SEVERITY_HIGH_KHR
#define CC_GL_DEBUG_SEVERITY_MEDIUM         GL_DEBUG_SEVERITY_MEDIUM_KHR
#define CC_GL_DEBUG_SEVERITY_LOW            GL_DEBUG_SEVERITY_LOW_KHR
#define CC_GL_DEBUG_SEVERITY_NOTIFICATION   GL_DEBUG_SEVERITY_NOTIFICATION_KHR
#else
#define CC_GL_DEBUG_APIENTRY                GLAPIENTRY
#define CC_GL_DEBUG_OUTPUT                  GL_DEBUG_OUTPUT
#define CC_GL_DEBUG_OUTPUT_SYNCHRONOUS      GL_DEBUG_OUTPUT_SYNCHRONOUS
#define CC_GL_DEBUG_SOURCE_API              GL_DEBUG_SOURCE_API
#define CC_GL_DEBUG_SOURCE_WINDOW_SYSTEM    GL_DEBUG_SOURCE_WINDOW_SYSTEM
#define CC_GL_DEBUG_SOURCE_SHADER_COMPILER  GL_DEBUG_SOURCE_SHADER_COMPILER
#define CC_GL_DEBUG_SOURCE_THIRD_PARTY      GL_DEBUG_SOURCE_THIRD_PARTY
#define CC_GL_DEBUG_SOURCE_APPLICATION      GL_DEBUG_SOURCE_APPLICATION
#define CC_GL_DEBUG_TYPE_ERROR              GL_DEBUG_TYPE_ERROR
#define CC_GL_DEBUG_TYPE_DEPRECATED         GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR
#define CC_GL_DEBUG_TYPE_UNDEFINED          GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR
#define CC_GL_DEBUG_TYPE_PORTABILITY        GL_DEBUG_TYPE_PORTABILITY
#define CC_GL_DEBUG_TYPE_PERFORMANCE        GL_DEBUG_TYPE_PERFORMANCE
#define CC_GL_DEBUG_TYPE_MARKER             GL_DEBUG_TYPE_MARKER
#define CC_GL_DEBUG_SEVERITY_HIGH           GL_DEBUG_SEVERITY_HIGH
#define CC_GL_DEBUG_SEVERITY_MEDIUM         GL_DEBUG_SEVERITY_MEDIUM
#define CC_GL_DEBUG_SEVERITY_LOW            GL_DEBUG_SEVERITY_LOW
#define CC_GL_DEBUG_SEVERITY_NOTIFICATION   GL_DEBUG_SEVERITY_NOTIFICATION
#endif
#endif

CC_BACKEND_BEGIN

#if CC_GL_DEBUG_OUTPUT_SUPPORTED
namespace
{
    const char* debugSourceName(GLenum source)
    {
        switch (source)
        {
        case CC_GL_DEBUG_SOURCE_API:                return "api";
        case CC_GL_DEBUG_SOURCE_WINDOW_SYSTEM:      return "window system";
        case CC_GL_DEBUG_SOURCE_SHADER_COMPILER:    return "shader compiler";
        case CC_GL_DEBUG_SOURCE_THIRD_PARTY:        return "third party";
        case CC_GL_DEBUG_SOURCE_APPLICATION:        return "application";
        default:                                    return "other";
        }
    }

    const char* debugTypeName(GLenum type)
    {
        switch (type)
        {
        case CC_GL_DEBUG_TYPE_ERROR:                return "error";
        case CC_GL_DEBUG_TYPE_DEPRECATED:           return "deprecated behaviour";
        case CC_GL_DEBUG_TYPE_UNDEFINED:            return "undefined behaviour";
        case CC_GL_DEBUG_TYPE_PORTABILITY:          return "portability";
        case CC_GL_DEBUG_TYPE_PERFORMANCE:          return "performance";
        case CC_GL_DEBUG_TYPE_MARKER:               return "marker";
        default:                                    return "other";
        }
    }

    const char* debugSeverityName(GLenum severity)
    {
        switch (severity)
        {
        case CC_GL_DEBUG_SEVERITY_HIGH:             return "HIGH";
        case CC_GL_DEBUG_SEVERITY_MEDIUM:           return "MEDIUM";
        case CC_GL_DEBUG_SEVERITY_LOW:              return "LOW";
        default:                                    return "INFO";
        }
    }

    void CC_GL_DEBUG_APIENTRY onDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity,
                                             GLsizei, const GLchar* message, const void*)
    {
        cocos2d::log("cocos2d: GL %s %s from %s (id %u): %s",
                     debugSeverityName(severity), debugTypeName(type), debugSourceName(source),
                     id, message ? message : "");
    }

    void enableDebugOutput(const std::string& extensions)
    {
#if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID
        if (extensions.find("GL_KHR_debug") == std::string::npos)
        {
            cocos2d::log("cocos2d: KHR_debug is unavailable, GL errors stay silent until the next glGetError()");
            return;
        }

        auto messageCallback = (PFNGLDEBUGMESSAGECALLBACKKHRPROC)eglGetProcAddress("glDebugMessageCallbackKHR");
        auto messageControl = (PFNGLDEBUGMESSAGECONTROLKHRPROC)eglGetProcAddress("glDebugMessageControlKHR");
#else
        CC_UNUSED_PARAM(extensions);
        auto messageCallback = (GLEW_VERSION_4_3 || GLEW_KHR_debug) ? glDebugMessageCallback : nullptr;
        auto messageControl = (GLEW_VERSION_4_3 || GLEW_KHR_debug) ? glDebugMessageControl : nullptr;
#endif
        if (!messageCallback)
        {
            cocos2d::log("cocos2d: KHR_debug is unavailable, GL errors stay silent until the next glGetError()");
            return;
        }

        glEnable(CC_GL_DEBUG_OUTPUT);
        glEnable(CC_GL_DEBUG_OUTPUT_SYNCHRONOUS);
        messageCallback(onDebugMessage, nullptr);
        if (messageControl)
            messageControl(GL_DONT_CARE, GL_DONT_CARE, CC_GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);

        cocos2d::log("cocos2d: GL debug output enabled");
    }
}
#endif

bool DeviceInfoGL::init()
{
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &_maxAttributes);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &_maxTextureUnits);
    _glExtensions = (const char*)glGetString(GL_EXTENSIONS);
#if CC_GL_DEBUG_OUTPUT_SUPPORTED
    enableDebugOutput(_glExtensions);
#endif
    return true;
}

const char* DeviceInfoGL::getVendor() const
{
    return (const char*)glGetString(GL_VENDOR);
}
const char* DeviceInfoGL::getRenderer() const
{
    return (const char*)glGetString(GL_RENDERER);
}

const char* DeviceInfoGL::getVersion() const
{
    return (const char*)glGetString(GL_VERSION);
}

const char* DeviceInfoGL::getExtension() const
{
    return _glExtensions.c_str();
}

bool DeviceInfoGL::checkForFeatureSupported(FeatureType feature)
{
    bool featureSupported = false;
    switch (feature)
    {
    case FeatureType::ETC1:
#ifdef GL_ETC1_RGB8_OES //GL_ETC1_RGB8_OES is not defined in old opengl version
        featureSupported = checkForGLExtension("GL_OES_compressed_ETC1_RGB8_texture");
#endif
        break;
    case FeatureType::ETC2:
    {
        GLint major = 0, minor = 0;
        glGetIntegerv(GL_MAJOR_VERSION, &major);
        glGetIntegerv(GL_MINOR_VERSION, &minor);
#ifdef CC_PLATFORM_PC
        featureSupported = major > 4 || (major == 4 && minor >= 3);
#else
        featureSupported = major >= 3;
#endif
        break;
    }
    case FeatureType::S3TC:
#ifdef GL_EXT_texture_compression_s3tc
        featureSupported = checkForGLExtension("GL_EXT_texture_compression_s3tc");
#endif
        break;
    case FeatureType::AMD_COMPRESSED_ATC:
        featureSupported = checkForGLExtension("GL_AMD_compressed_ATC_texture");
        break;
    case FeatureType::PVRTC:
        featureSupported = checkForGLExtension("GL_IMG_texture_compression_pvrtc");
        break;
    case FeatureType::IMG_FORMAT_BGRA8888:
        featureSupported = checkForGLExtension("GL_IMG_texture_format_BGRA8888");
        break;
    case FeatureType::DISCARD_FRAMEBUFFER:
        featureSupported = checkForGLExtension("GL_EXT_discard_framebuffer");
        break;
    case FeatureType::PACKED_DEPTH_STENCIL:
        featureSupported = checkForGLExtension("GL_OES_packed_depth_stencil");
        break;
    case FeatureType::VAO:
#ifdef CC_PLATFORM_PC
        featureSupported = checkForGLExtension("vertex_array_object");
#else
        featureSupported = checkForGLExtension("GL_OES_vertex_array_object");
#endif
        break;
    case FeatureType::MAPBUFFER:
        featureSupported = checkForGLExtension("GL_OES_mapbuffer");
        break;
    case FeatureType::DEPTH24:
        featureSupported = checkForGLExtension("GL_OES_depth24");
        break;
    default:
        break;
    }
    return featureSupported;
}

bool DeviceInfoGL::checkForGLExtension(const std::string &searchName) const
{
    return _glExtensions.find(searchName) != std::string::npos;
}

CC_BACKEND_END
