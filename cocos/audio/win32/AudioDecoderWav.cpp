/****************************************************************************
 Copyright (c) 2026 Vladislav Kochetkov

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

#include "audio/win32/AudioDecoderWav.h"
#include "audio/win32/AudioMacros.h"
#include "platform/CCFileUtils.h"

#include "base/CCConsole.h"

#include <algorithm>
#include <string.h>

#define LOG_TAG "AudioDecoderWav"

namespace cocos2d {

    namespace {

        const uint16_t kFormatPcm = 0x0001;
        const uint16_t kFormatExtensible = 0xFFFE;

        bool readChunkHeader(FILE* file, char id[ 4 ], uint32_t& size)
        {
            return fread(id, 1, 4, file) == 4 && fread(&size, 4, 1, file) == 1;
        }

        template <typename T>
        bool readValue(FILE* file, T& value)
        {
            return fread(&value, sizeof(T), 1, file) == 1;
        }

    } // namespace

    AudioDecoderWav::AudioDecoderWav()
        : m_file(nullptr)
        , m_dataOffset(0)
        , m_currentFrame(0)
    {
    }

    AudioDecoderWav::~AudioDecoderWav()
    {
        close();
    }

    bool AudioDecoderWav::open(const char* path)
    {
        std::string fullPath = FileUtils::getInstance()->fullPathForFilename(path);

        m_file = fopen(FileUtils::getInstance()->getSuitableFOpen(fullPath).c_str(), "rb");
        if (m_file == nullptr)
        {
            ALOGE("Failed to open %s", fullPath.c_str());
            return false;
        }

        if (!parseHeader(fullPath))
        {
            close();
            return false;
        }

        m_currentFrame = 0;
        _isOpened = true;
        return true;
    }

    bool AudioDecoderWav::parseHeader(const std::string& fullPath)
    {
        char riff[ 12 ];
        if (fread(riff, 1, 12, m_file) != 12
            || memcmp(riff, "RIFF", 4) != 0
            || memcmp(riff + 8, "WAVE", 4) != 0)
        {
            ALOGE("%s is not a RIFF/WAVE file", fullPath.c_str());
            return false;
        }

        if (fseek(m_file, 0, SEEK_END) != 0)
            return false;
        const long fileSize = ftell(m_file);
        if (fseek(m_file, 12, SEEK_SET) != 0)
            return false;

        uint16_t audioFormat = 0;
        uint16_t channelCount = 0;
        uint16_t bitsPerSample = 0;
        uint32_t sampleRate = 0;
        uint32_t dataSize = 0;
        bool hasFormat = false;

        char chunkId[ 4 ];
        uint32_t chunkSize = 0;

        while (readChunkHeader(m_file, chunkId, chunkSize))
        {
            const long chunkStart = ftell(m_file);

            if (memcmp(chunkId, "fmt ", 4) == 0 && chunkSize >= 16)
            {
                uint16_t blockAlign = 0;
                uint32_t byteRate = 0;
                if (!readValue(m_file, audioFormat) || !readValue(m_file, channelCount)
                    || !readValue(m_file, sampleRate) || !readValue(m_file, byteRate)
                    || !readValue(m_file, blockAlign) || !readValue(m_file, bitsPerSample))
                {
                    return false;
                }

                if (audioFormat == kFormatExtensible && chunkSize >= 40)
                {
                    uint16_t subFormat = 0;
                    if (fseek(m_file, chunkStart + 24, SEEK_SET) != 0 || !readValue(m_file, subFormat))
                        return false;
                    audioFormat = subFormat;
                }

                hasFormat = true;
            }
            else if (memcmp(chunkId, "data", 4) == 0)
            {
                const long available = fileSize - chunkStart;
                if (available <= 0)
                    break;

                m_dataOffset = chunkStart;
                dataSize = std::min<uint32_t>(chunkSize, static_cast<uint32_t>(available));
                break;
            }

            if (fseek(m_file, chunkStart + chunkSize + (chunkSize & 1), SEEK_SET) != 0)
                return false;
        }

        if (!hasFormat || dataSize == 0)
        {
            ALOGE("%s has no usable 'fmt ' or 'data' chunk", fullPath.c_str());
            return false;
        }

        if (audioFormat != kFormatPcm || bitsPerSample != 16 || channelCount < 1 || channelCount > 2)
        {
            ALOGE("%s is not 16 bit PCM mono/stereo (format 0x%04X, %u bits, %u channels), unsupported",
                  fullPath.c_str(), audioFormat, bitsPerSample, channelCount);
            return false;
        }

        _channelCount = channelCount;
        _sampleRate = sampleRate;
        _bytesPerFrame = channelCount * sizeof(int16_t);
        _totalFrames = dataSize / _bytesPerFrame;

        return _totalFrames > 0 && fseek(m_file, m_dataOffset, SEEK_SET) == 0;
    }

    void AudioDecoderWav::close()
    {
        if (m_file != nullptr)
        {
            fclose(m_file);
            m_file = nullptr;
        }
        _isOpened = false;
    }

    uint32_t AudioDecoderWav::read(uint32_t framesToRead, char* pcmBuf)
    {
        if (!isOpened() || m_currentFrame >= _totalFrames)
            return 0;

        const uint32_t frames = std::min(framesToRead, _totalFrames - m_currentFrame);
        const size_t bytesRead = fread(pcmBuf, 1, frames * _bytesPerFrame, m_file);
        const uint32_t framesRead = static_cast<uint32_t>(bytesRead / _bytesPerFrame);

        m_currentFrame += framesRead;
        return framesRead;
    }

    bool AudioDecoderWav::seek(uint32_t frameOffset)
    {
        if (!isOpened() || frameOffset > _totalFrames)
            return false;

        if (fseek(m_file, m_dataOffset + static_cast<long>(frameOffset) * _bytesPerFrame, SEEK_SET) != 0)
            return false;

        m_currentFrame = frameOffset;
        return true;
    }

    uint32_t AudioDecoderWav::tell() const
    {
        return m_currentFrame;
    }

} // namespace cocos2d {
