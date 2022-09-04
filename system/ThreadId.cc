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

#include "system/ThreadId.h"

#include "base/Likely.h"
#include <pthread.h>
#include <sys/syscall.h>
#include <utility>
#include <unistd.h>
#include <atomic>
#include "system/AtFork.h"

namespace tinylog
{
    uint64_t getCurrentThreadID()
    {
        return uint64_t(pthread_self());
    }

    namespace detail
    {
        uint64_t getOSThreadIDSlow()
        {
            return uint64_t(syscall(SYS_gettid));
        }

    } // namespace detail

    namespace
    {
        struct CacheState
        {
            CacheState()
            {
                AtFork::registerHandler(
                    this, [] { return true; }, [] {}, [] { ++epoch; });
            }

            // Used to invalidate all caches in the child process on fork. Start at 1 so
            // that 0 is always invalid.
            static std::atomic<uint64_t> epoch; 
        };

        std::atomic<uint64_t> CacheState::epoch{1};

        CacheState gCacheState;

    } // namespace

    uint64_t getOSThreadID()
    {
        thread_local std::pair<uint64_t, uint64_t> cache{0, 0};
        auto epoch = CacheState::epoch.load();
        if (UNLIKELY(epoch != cache.first))
        {
            cache = {epoch, detail::getOSThreadIDSlow()};
        }
        return cache.second;
    }

} // namespace tinylog
