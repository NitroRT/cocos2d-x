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
 
#pragma once

#include "Macros.h"
#include "Types.h"
#include "VertexLayout.h"
#include "renderer/backend/ProgramState.h"

#include <vector>
#include <memory>

CC_BACKEND_BEGIN

class DepthStencilState;
class BlendState;

struct RenderPipelineDescriptor
{
    ProgramState* programState = nullptr;
    BlendState* blendState = nullptr;
    std::shared_ptr<std::vector<VertexLayout>> vertexLayouts = std::make_shared<std::vector<VertexLayout>>();
    PixelFormat colorAttachmentsFormat[MAX_COLOR_ATTCHMENT] = { PixelFormat::DEFAULT };
    PixelFormat depthAttachmentFormat = PixelFormat::NONE;
    PixelFormat stencilAttachmentFormat = PixelFormat::NONE;
};

CC_BACKEND_END