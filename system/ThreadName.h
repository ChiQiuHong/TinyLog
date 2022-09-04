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

#include <string>
#include <thread>

#include <optional>
#include "include/StringPiece.h"
#include <pthread.h>

namespace tinylog
{
    /**
     * This returns true if the current platform supports setting the name of the
     * current thread.
     */
    bool canSetCurrentThreadName();

    /**
     * This returns true if the current platform supports setting the name of
     * threads other than the one currently executing.
     */
    bool canSetOtherThreadName();

    /**
     * Get the name of the given thread, or nothing if an error occurs
     * or the functionality is not available.
     */
    std::optional<std::string> getThreadName(std::thread::id tid);

    /**
     * Equivalent to getThreadName(std::this_thread::get_id());
     */
    std::optional<std::string> getCurrentThreadName();

    /**
     * Set the  name of the given thread.
     * Returns false on failure, if an error occurs or the functionality
     * is not available.
     */
    bool setThreadName(std::thread::id tid, StringPiece name);
    bool setThreadName(pthread_t pid, StringPiece name);

    /**
     * Equivalent to setThreadName(std::this_thread::get_id(), name);
     */
    bool setThreadName(StringPiece name);

} // namespace tinylog
