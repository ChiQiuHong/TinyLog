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

#include "system/ThreadName.h"

#include <type_traits>

#if defined(__GLIBC__)
#if __GLIBC_PREREQ(2, 12)
#define HAS_PTHREAD_SETNAME_NP_THREAD_NAME 1
#else
#define HAS_PTHREAD_SETNAME_NP_THREAD_NAME 0
#endif
#endif

namespace tinylog
{

    namespace
    {

        pthread_t stdTidToPthreadId(std::thread::id tid)
        {
            static_assert(
                std::is_same<pthread_t, std::thread::native_handle_type>::value,
                "This assumes that the native handle type is pthread_t");
            static_assert(
                sizeof(std::thread::native_handle_type) == sizeof(std::thread::id),
                "This assumes std::thread::id is a thin wrapper around "
                "std::thread::native_handle_type, but that doesn't appear to be true.");
            // In most implementations, std::thread::id is a thin wrapper around
            // std::thread::native_handle_type, which means we can do unsafe things to
            // extract it.
            pthread_t id;
            std::memcpy(&id, &tid, sizeof(id));
            return id;
        }

    } // namespace

    bool canSetCurrentThreadName()
    {
#ifdef HAS_PTHREAD_SETNAME_NP_THREAD_NAME
        return true;
#else
        return false;
#endif
    }

    bool canSetOtherThreadName()
    {
#ifdef HAS_PTHREAD_SETNAME_NP_THREAD_NAME
        return true;
#else
        return false;
#endif
    }

    static constexpr size_t kMaxThreadNameLength = 16;

    static std::optional<std::string> getPThreadName(pthread_t pid)
    {
#ifdef HAS_PTHREAD_SETNAME_NP_THREAD_NAME
        std::array<char, kMaxThreadNameLength> buf;
        if (pthread_getname_np(pid, buf.data(), buf.size()) == 0)
        {
            return std::string(buf.data());
        }
#endif
        (void)pid;
        return std::nullopt;
    }

    std::optional<std::string> getThreadName(std::thread::id id)
    {
#ifdef HAS_PTHREAD_SETNAME_NP_THREAD_NAME
        if (id == std::thread::id())
        {
            return getPThreadName(stdTidToPthreadId(id));
        }
#endif
        (void)id;
        return std::nullopt;
    }

    std::optional<std::string> getCurrentThreadName()
    {
        return getPThreadName(pthread_self());
    }

    bool setThreadName(std::thread::id tid, StringPiece name)
    {
        return setThreadName(stdTidToPthreadId(tid), name);
    }

    bool setThreadName(pthread_t pid, StringPiece name)
    {
        name = name.subpiece(0, kMaxThreadNameLength - 1);
        char buf[kMaxThreadNameLength] = {};
        std::memcpy(buf, name.data(), name.size());
#if HAS_PTHREAD_SETNAME_NP_THREAD_NAME
        return 0 == pthread_setname_np(pid, buf);
#endif
        (void)pid;
        return false;
    }

    bool setThreadName(StringPiece name)
    {
        return setThreadName(pthread_self(), name);
    }

} // namespace tinylog
