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

#include "LogLevel.h"

#include <array>
#include <cctype>
#include <ostream>

using std::string;

namespace tinylog
{
    LogLevel stringToLogLevel(StringPiece name)
    {
        string lowerNameStr;
        lowerNameStr.reserve(name.size());
        for (char c : name)
        {
            lowerNameStr.push_back(static_cast<char>(std::tolower(c)));
        }
        StringPiece lowerName(lowerNameStr);

        // If the string is of the form "LogLevel::foo" or "LogLevel(foo)"
        // strip it down just to "foo". This makes sure we can process both
        // the "LogLevel::WARN" and "LogLevel(1234)" formats produced by
        // logLevelToString().
        constexpr StringPiece lowercasePrefix{"loglevel::"};
        constexpr StringPiece wrapperPrefix{"loglevel("};
        if (lowerName.startsWith(lowercasePrefix))
        {
            lowerName.remove_prefix(lowercasePrefix.size());
        }
        else if (lowerName.startsWith(wrapperPrefix) && lowerName.endsWith(")"))
        {
            lowerName.remove_prefix(wrapperPrefix.size());
            lowerName.subtract(1);
        }

        if (lowerName == "uninitialized")
        {
            return LogLevel::UNINITIALIZED;
        }
        else if (lowerName == "none")
        {
            return LogLevel::NONE;
        }
        else if (lowerName == "debug" || lowerName == "dbg")
        {
            return LogLevel::DBG;
        }
        else if (lowerName == "info")
        {
            return LogLevel::INFO;
        }
        else if (lowerName == "warn" || lowerName == "warning")
        {
            return LogLevel::WARN;
        }
        else if (lowerName == "error" || lowerName == "err")
        {
            return LogLevel::ERROR;
        }
        else if (lowerName == "critical")
        {
            return LogLevel::CRITICAL;
        }
        else if (lowerName == "dfatal")
        {
            return LogLevel::DFATAL;
        }
        else if (lowerName == "fatal")
        {
            return LogLevel::FATAL;
        }
        else if (lowerName == "max" || lowerName == "max_level")
        {
            return LogLevel::MAX_LEVEL;
        }
    }

    string logLevelToString(LogLevel level)
    {
        if (level == LogLevel::UNINITIALIZED)
        {
            return "UNINITIALIZED";
        }
        else if (level == LogLevel::NONE)
        {
            return "NONE";
        }
        else if (level == LogLevel::DBG)
        {
            return "DEBUG";
        }
        else if (level == LogLevel::INFO)
        {
            return "INFO";
        }
        else if (level == LogLevel::WARN)
        {
            return "WARN";
        }
        else if (level == LogLevel::ERROR)
        {
            return "ERROR";
        }
        else if (level == LogLevel::CRITICAL)
        {
            return "CRITICAL";
        }
        else if (level == LogLevel::DFATAL)
        {
            return "DFATAL";
        }
        else if (level == LogLevel::FATAL)
        {
            return "FATAL";
        }
    }

    std::ostream &operator<<(std::ostream &os, LogLevel level)
    {
        os << logLevelToString(level);
        return os;
    }
} // namespace tinylog
