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

#include "system/AtFork.h"

#include <atomic>
#include <mutex>
#include <thread>

#include <glog/logging.h>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

class AtForkListTest : public Test
{
};

TEST_F(AtForkListTest, append_nullptr)
{
    tinylog::AtForkList list;
    EXPECT_FALSE(list.contains(nullptr));
    list.append(nullptr, nullptr, nullptr, nullptr);
    EXPECT_FALSE(list.contains(nullptr));
}

TEST_F(AtForkListTest, remove_nullptr)
{
    tinylog::AtForkList list;
    EXPECT_FALSE(list.contains(nullptr));
    list.remove(nullptr);
    EXPECT_FALSE(list.contains(nullptr));
}

TEST_F(AtForkListTest, append_key)
{
    tinylog::AtForkList list;
    char key;
    EXPECT_FALSE(list.contains(&key));
    list.append(&key, nullptr, nullptr, nullptr);
    EXPECT_TRUE(list.contains(&key));
}

TEST_F(AtForkListTest, append_key_duplicate)
{
    tinylog::AtForkList list;
    char key;
    EXPECT_FALSE(list.contains(&key));
    list.append(&key, nullptr, nullptr, nullptr);
    EXPECT_TRUE(list.contains(&key));
    EXPECT_THROW(
        list.append(&key, nullptr, nullptr, nullptr), std::invalid_argument);
}

TEST_F(AtForkListTest, remove_key)
{
    tinylog::AtForkList list;
    char key;
    EXPECT_FALSE(list.contains(&key));
    list.append(&key, nullptr, nullptr, nullptr);
    EXPECT_TRUE(list.contains(&key));
    list.remove(&key);
    EXPECT_FALSE(list.contains(&key));
}

TEST_F(AtForkListTest, remove_key_missing)
{
    tinylog::AtForkList list;
    char key;
    EXPECT_FALSE(list.contains(&key));
    EXPECT_THROW(list.remove(&key), std::out_of_range);
}

TEST_F(AtForkListTest, prepare_parent)
{
    tinylog::AtForkList list;
    std::vector<int> ops;
    list.append(
        nullptr,
        [&]
        { return ops.push_back(70), true; },
        [&]
        { ops.push_back(80); },
        [&]
        { ops.push_back(90); });
    list.prepare();
    list.parent();
    EXPECT_THAT(ops, ElementsAre(70, 80));
}

TEST_F(AtForkListTest, prepare_parent_prepare_empty)
{
    tinylog::AtForkList list;
    std::vector<int> ops;
    list.append(
        nullptr, //
        nullptr,
        [&]
        { ops.push_back(80); },
        [&]
        { ops.push_back(90); });
    list.prepare();
    list.parent();
    EXPECT_THAT(ops, ElementsAre(80));
}

TEST_F(AtForkListTest, prepare_parent_parent_empty)
{
    tinylog::AtForkList list;
    std::vector<int> ops;
    list.append(
        nullptr, //
        [&]
        { return ops.push_back(70), true; },
        nullptr,
        [&]
        { ops.push_back(90); });
    list.prepare();
    list.parent();
    EXPECT_THAT(ops, ElementsAre(70));
}

TEST_F(AtForkListTest, prepare_parent_multi)
{
    tinylog::AtForkList list;
    std::vector<int> ops;
    for (size_t i = 0; i != 3; ++i)
    {
        list.append(
            nullptr,
            [&, i]
            { return ops.push_back(70 + i), true; },
            [&, i]
            { ops.push_back(80 + i); },
            [&, i]
            { ops.push_back(90 + i); });
    }
    list.prepare();
    list.parent();
    EXPECT_THAT(ops, ElementsAre(72, 71, 70, 80, 81, 82));
}

TEST_F(AtForkListTest, prepare_parent_return_false)
{
    tinylog::AtForkList list;
    std::vector<int> ops;
    for (size_t i = 0; i != 4; ++i)
    {
        list.append(
            nullptr,
            [&, i, q = 0]() mutable
            { return ops.push_back(70 + i), q++ || i != 1; },
            [&, i]
            { ops.push_back(80 + i); },
            [&, i]
            { ops.push_back(90 + i); });
    }
    list.prepare();
    list.parent();
    EXPECT_THAT(ops, ElementsAre(73, 72, 71, 83, 82, 73, 72, 71, 70, 80, 81, 82, 83));
}

TEST_F(AtForkListTest, prepare_child)
{
    tinylog::AtForkList list;
    std::vector<int> ops;
    list.append(
        nullptr,
        [&]
        { return ops.push_back(70), true; },
        [&]
        { ops.push_back(80); },
        [&]
        { ops.push_back(90); });
    list.prepare();
    list.child();
    EXPECT_THAT(ops, ElementsAre(70, 90));
}

TEST_F(AtForkListTest, prepare_child_prepare_empty)
{
    tinylog::AtForkList list;
    std::vector<int> ops;
    list.append(
        nullptr, //
        [&]
        { return ops.push_back(70), true; },
        nullptr,
        [&]
        { ops.push_back(90); });
    list.prepare();
    list.child();
    EXPECT_THAT(ops, ElementsAre(70, 90));
}

TEST_F(AtForkListTest, prepare_child_child_empty)
{
    tinylog::AtForkList list;
    std::vector<int> ops;
    list.append(
        nullptr, //
        [&]
        { return ops.push_back(70), true; },
        [&]
        { ops.push_back(80); },
        nullptr);
    list.prepare();
    list.child();
    EXPECT_THAT(ops, ElementsAre(70));
}

TEST_F(AtForkListTest, prepare_child_multi)
{
    tinylog::AtForkList list;
    std::vector<int> ops;
    for (size_t i = 0; i != 3; ++i)
    {
        list.append(
            nullptr,
            [&, i]
            { return ops.push_back(70 + i), true; },
            [&, i]
            { ops.push_back(80 + i); },
            [&, i]
            { ops.push_back(90 + i); });
    }
    list.prepare();
    list.child();
    EXPECT_THAT(ops, ElementsAre(72, 71, 70, 90, 91, 92));
}

TEST_F(AtForkListTest, prepare_child_return_false)
{
    tinylog::AtForkList list;
    std::vector<int> ops;
    for (size_t i = 0; i != 4; ++i)
    {
        list.append(
            nullptr,
            [&, i, q = 0]() mutable
            { return ops.push_back(70 + i), q++ || i != 1; },
            [&, i]
            { ops.push_back(80 + i); },
            [&, i]
            { ops.push_back(90 + i); });
    }
    list.prepare();
    list.child();
    EXPECT_THAT(ops, ElementsAre(73, 72, 71, 83, 82, 73, 72, 71, 70, 90, 91, 92, 93));
}

class AtForkTest : public Test
{
};

TEST_F(AtForkTest, prepare)
{
    int foo;
    bool forked = false;
    tinylog::AtFork::registerHandler(
        &foo,
        [&]
        { forked = true; return true; },
        [] {},
        [] {});
    auto pid = tinylog::AtFork::forkInstrumented(fork);
    PCHECK(pid != -1);
    if (pid)
    {
        int status;
        auto pid2 = waitpid(pid, &status, 0);
        EXPECT_EQ(status, 0);
        EXPECT_EQ(pid, pid2);
    }
    else
    {
        _exit(0);
    }
    EXPECT_TRUE(forked);
    forked = false;
    tinylog::AtFork::unregisterHandler(&foo);
    pid = fork();
    PCHECK(pid != -1);
    if (pid)
    {
        int status;
        auto pid2 = waitpid(pid, &status, 0);
        EXPECT_EQ(status, 0);
        EXPECT_EQ(pid, pid2);
    }
    else
    {
        _exit(0);
    }
    EXPECT_FALSE(forked);
}

TEST_F(AtForkTest, ordering)
{
    std::atomic<bool> started{false};
    std::mutex a;
    std::mutex b;
    int foo;
    int foo2;
    tinylog::AtFork::registerHandler(
        &foo,
        [&]
        { return a.try_lock(); },
        [&]
        { b.unlock(); },
        [&]
        { b.unlock(); });

    tinylog::AtFork::registerHandler(
        &foo2,
        [&]
        { return b.try_lock(); },
        [&]
        { b.unlock(); },
        [&]
        { b.unlock(); });

    auto thr = std::thread([&]()
                           {
        std::lock_guard<std::mutex> g(a);
        started = true;
        usleep(100);
        std::lock_guard<std::mutex> g2(b); });
    while (!started)
    {
    }
    auto pid = tinylog::AtFork::forkInstrumented(fork);
    PCHECK(pid != -1);
    if (pid)
    {
        int status;
        auto pid2 = waitpid(pid, &status, 0);
        EXPECT_TRUE(WIFEXITED(status));
        EXPECT_THAT(
            WEXITSTATUS(status),
            AnyOfArray({0, 66}));
        EXPECT_EQ(pid, pid2);
    }
    else
    {
        _exit(0);
    }
    thr.join();
}