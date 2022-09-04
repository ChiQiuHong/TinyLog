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

#include <thread>
#include <gtest/gtest.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace tinylog
{
    namespace detail
    {
        uint64_t getOSThreadIDSlow();
    } // namespace detail

} // namespace tinylog

namespace
{
    template <class F>
    void testUnique(F &&f)
    {
        auto thisThreadID = f();
        uint64_t otherThreadID;
        std::thread otherThread{[&] { otherThreadID = f(); }};
        otherThread.join();
        EXPECT_NE(thisThreadID, otherThreadID);
    }

} // namespace

TEST(ThreadId, getCurrentID)
{
    testUnique(tinylog::getCurrentThreadID);
}

TEST(ThreadId, getOSThreadID)
{
    testUnique(tinylog::getOSThreadID);
}

TEST(ThreadId, getOSThreadIDCache)
{
    auto thisThreadID = tinylog::getOSThreadID();
    ASSERT_EQ(thisThreadID, tinylog::detail::getOSThreadIDSlow());

    auto pid = fork();
    ASSERT_GE(pid, 0);
    if (pid == 0)   // Child.
    {
        if (tinylog::getOSThreadID() != tinylog::detail::getOSThreadIDSlow())
        {
            _exit(1);
        }
        if (tinylog::getOSThreadID() == thisThreadID)
        {
            _exit(2);
        }
        _exit(0);
    }
    else // Parent
    {
        int status;
        ASSERT_EQ(pid, waitpid(pid, &status, 0));
        ASSERT_TRUE(WIFEXITED(status));
        EXPECT_EQ(0, WEXITSTATUS(status));
    } 
}
