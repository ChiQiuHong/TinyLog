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

#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>
#include <type_traits>

#include "Portability.h"
#include "StringPiece.h"

namespace tinylog
{
    /**
     * Log Level values.
     *
     * Higher levels are more important than lower ones.
     */
    enum class LogLevel : uint32_t
    {
        UNINITIALIZED = 0,
        NONE = 1,
        MIN_LEVEL = 1,

        // "DBG" is the lowest (aka most verbose) debug log level.
        // This level is intended to be permarily used in log category settings.
        //
        // This is named "DBG" rather than "DEBUG" since some open source projects
        // define "DBUG" as a apreprocessor macro.
        DBG = 1000,

        INFO = 2000,

        WARN = 3000,
        WARNING = 3000,

        // Unfortunately Windows headers #define ERROR, so we cannot use
        // it as an enum value name. We only provide ERR instead.
        // But TinyLog is only use in Linux, so can use ERROR.
        ERROR = 4000,

        CRITICAL = 5000,

        // DFATAL log messages crash the program on debug builds.
        DFATAL = 0x7ffffffe,
        // FATAL log messages always abort the program.
        // This level is equivalent to MAX_LEVEL.
        FATAL = 0x7fffffff,

        // The most significant bit is used by LogCategory to store a flag value,
        // so the maximum value has that bit cleared.
        //
        // (We call this MAX_LEVEL instead of MAX just since MAX() is commonly
        // defined as a preprocessor macro by some C headers.)
        MAX_LEVEL = 0x7fffffff,
    };

    constexpr LogLevel kDefaultLogLevel = LogLevel::INFO;

    /*
     * Support adding and subtracting integers from LogLevels, to create slightly
     * adjusted log level values.
     */
    inline constexpr LogLevel operator+(LogLevel level, uint32_t value)
    {
        // Cap the result at LogLevel::MAX_LEVEL
        return ((static_cast<uint32_t>(level) + value) >
                static_cast<uint32_t>(LogLevel::MAX_LEVEL))
                   ? LogLevel::MAX_LEVEL
                   : static_cast<LogLevel>(static_cast<uint32_t>(level) + value);
    }

    inline LogLevel &operator+=(LogLevel &level, uint32_t value)
    {
        level = level + value;
        return level;
    }

    inline constexpr LogLevel operator-(LogLevel level, uint32_t value)
    {
        return static_cast<LogLevel>(static_cast<uint32_t>(level) - value);
    }

    inline LogLevel &operator-=(LogLevel &level, uint32_t value)
    {
        level = level - value;
        return level;
    }

    /**
     * Construct a LogLevel from a string name.
     */
    LogLevel stringToLogLevel(tinylog::StringPiece name);

    /**
     * Get a human-readable string representing the logLevel.
     */
    std::string logLevelToString(LogLevel level);

    /**
     * Print a LogLevel in a human readable format.
     */
    std::ostream &operator<<(std::ostream &os, LogLevel level);

    /**
     * Returns true if and only if a LogLevel is fatal.
     */
    inline constexpr bool isLogLevelFatal(LogLevel level)
    {
        return tinylog::kIsDebug ? (level >= LogLevel::DFATAL)
                                 : (level >= LogLevel::FATAL);
    }

} // namespace tinylog
