/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "LogMessage.h"

#include "LogCategory.h"

using std::chrono::system_clock;

namespace
{
    std::string getContextStringFromCategory(const tinylog::LogCategory *category)
    {
        return category->getDB()->getContextString();
    }
}

namespace tinylog
{
    LogMessage::LogMessage(
        const LogCategory *category,
        LogLevel level,
        StringPiece filename,
        unsigned int lineNumber,
        StringPiece functionName,
        std::string &&msg)
        : category_{category},
          level_{level},
          threadID_{getOSThreadID()},
          timestamp_{system_clock::now()},
          filename_{filename},
          lineNumber_{lineNumber},
          functionName_{functionName},
          contextString_{getContexStringFromCategory(category_)},
          rawMessage_{std::move(msg)}
    {
        sanitizeMessage();
    }

    LogMessage::LogMessage(
        const LogCategory *category,
        LogLevel level,
        system_clock::time_point timestamp,
        StringPiece filename,
        unsigned int lineNumber,
        StringPiece functionName,
        std::string &&msg)
        : category_{category},
          level_{level},
          threadID_{getOSThreadID()},
          timestamp_{timestamp},
          filename_{filename},
          lineNumber_{lineNumber},
          functionName_{functionName},
          contextString_{getContexStringFromCategory(category_)},
          rawMessage_{std::move(msg)}
    {
        sanitizeMessage();
    }

    StringPiece LogMessage::getFileBaseName() const
    {
        auto idx = filename_.rfind('/');
        if (idx == std::string::npos)
        {
            return filename_;
        }
        return filename_.subpiece(idx + 1);
    }

    void LogMessage::sanitizeMessage()
    {
        // Compute how long the sanitized string will be.
        size_t sanitizedLength = 0;
        size_t numNewlines = 0;
        for (const char c : rawMessage_)
        {
            if (static_cast<unsigned char>(c) < 0x20)
            {
                // Newlines and tabs are emitted directly with no escaping.
                // All other control characters are emitted as \xNN (4 characters)
                if (c == '\n')
                {
                    sanitizedLength += 1;
                    ++numNewlines;
                }
                else if (c == '\t')
                {
                    sanitizedLength += 1;
                }
                else
                {
                    sanitizedLength += 4;
                }
            }
            else if (c == 0x7f)
            {
                // Bytes above the ASCII range are emitted as \xNN (4 characters)
                sanitizedLength += 4;
            }
            else
            {
                // This character will be emitted as-is, with no escaping.
                ++sanitizedLength;
            }
        }
        numNewlines_ = numNewlines;
        // If nothing is different, just use rawMessage_ directly,
        // and don't populate message_.
        if (sanitizedLength == rawMessage_.size())
        {
            return;
        }

        message_.reserve(sanitizedLength);
        for (const char c : rawMessage_)
        {
            if (static_cast<unsigned char>(c) < 0x20)
            {
                if (c == '\n' || c == '\t')
                {
                    message_.push_back(c);
                }
                else
                {
                    static constexpr StringPiece hexdigits{"0123456789abcdef"};
                    std::array<char, 4> data{
                        {'\\', 'x', hexdigits[(c >> 4) & 0xf], hexdigits[c & 0xf]}};
                    message_.append(data.data(), data.size());
                }
            }
            else if (c == 0x7f)
            {
                constexpr std::array<char, 4> data{{'\\', 'x', '7', 'f'}};
                message_.append(data.data(), data.size());
            }
            else
            {
                message_.push_back(c);
            }
        }
    }

} // namespace tinylog
