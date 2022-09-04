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

#include <thread>
#include <gtest/gtest.h>

using namespace std;
using namespace tinylog;

namespace
{

    const bool expectedSetOtherThreadNameResult = tinylog::canSetOtherThreadName();
    const bool expectedSetSelfThreadNameResult = tinylog::canSetCurrentThreadName();
    constexpr StringPiece kThreadName{"rockin-thread"};

} // namespace

TEST(ThreadName, getCurrentThreadName)
{
    thread th([] {
        EXPECT_EQ(expectedSetSelfThreadNameResult, setThreadName(kThreadName));
        if (expectedSetSelfThreadNameResult)
        {
            EXPECT_EQ(kThreadName.str(), *getCurrentThreadName());
        }
    });

    th.join();
}

// TEST(ThreadName, setThreadName_other_pthread)
// {

// }