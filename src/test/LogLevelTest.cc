#include "LogLevel.h"

#include <gtest/gtest.h>

using namespace tinylog;

TEST(LogLevel, fromString)
{
    EXPECT_EQ(LogLevel::UNINITIALIZED, stringToLogLevel("uninitialized"));
    EXPECT_EQ(LogLevel::UNINITIALIZED, stringToLogLevel("UnInitialized"));
    EXPECT_EQ(LogLevel::UNINITIALIZED, stringToLogLevel("LogLevel::UNINITIALIZED"));

    EXPECT_EQ(LogLevel::NONE, stringToLogLevel("none"));
    EXPECT_EQ(LogLevel::NONE, stringToLogLevel("NONE"));
    EXPECT_EQ(LogLevel::NONE, stringToLogLevel("NoNe"));
    EXPECT_EQ(LogLevel::NONE, stringToLogLevel("LogLevel::none"));

    EXPECT_EQ(LogLevel::DBG, stringToLogLevel("debug"));
    EXPECT_EQ(LogLevel::DBG, stringToLogLevel("dEBug"));
    EXPECT_EQ(LogLevel::DBG, stringToLogLevel("Dbg"));
    EXPECT_EQ(LogLevel::DBG, stringToLogLevel("loglevel::dEBug"));
    EXPECT_EQ(LogLevel::DBG, stringToLogLevel("loglevel::DBG"));

    EXPECT_EQ(LogLevel::INFO, stringToLogLevel("info"));
    EXPECT_EQ(LogLevel::INFO, stringToLogLevel("INFO"));
    EXPECT_EQ(LogLevel::INFO, stringToLogLevel("loglevel(INFO)"));

    EXPECT_EQ(LogLevel::WARN, stringToLogLevel("warn"));
    EXPECT_EQ(LogLevel::WARN, stringToLogLevel("WARN"));
    EXPECT_EQ(LogLevel::WARN, stringToLogLevel("warning"));

    EXPECT_EQ(LogLevel::ERROR, stringToLogLevel("err"));
    EXPECT_EQ(LogLevel::ERROR, stringToLogLevel("eRr"));
    EXPECT_EQ(LogLevel::ERROR, stringToLogLevel("error"));
    EXPECT_EQ(LogLevel::ERROR, stringToLogLevel("ERR"));
    EXPECT_EQ(LogLevel::ERROR, stringToLogLevel("ERROR"));

    EXPECT_EQ(LogLevel::CRITICAL, stringToLogLevel("critical"));
    EXPECT_EQ(LogLevel::CRITICAL, stringToLogLevel("CRITICAL"));

    EXPECT_EQ(LogLevel::DFATAL, stringToLogLevel("dfatal"));
    EXPECT_EQ(LogLevel::DFATAL, stringToLogLevel("DFatal"));
    EXPECT_EQ(LogLevel::DFATAL, stringToLogLevel("DFATAL"));

    EXPECT_EQ(LogLevel::FATAL, stringToLogLevel("fatal"));
    EXPECT_EQ(LogLevel::FATAL, stringToLogLevel("Fatal"));
    EXPECT_EQ(LogLevel::FATAL, stringToLogLevel("FATAL"));

    EXPECT_EQ(LogLevel::MAX_LEVEL, stringToLogLevel("max"));
    EXPECT_EQ(LogLevel::MAX_LEVEL, stringToLogLevel("Max_Level"));
    EXPECT_EQ(LogLevel::MAX_LEVEL, stringToLogLevel("LogLevel::MAX"));
    EXPECT_EQ(LogLevel::MAX_LEVEL, stringToLogLevel("LogLevel::MAX_LEVEL"));
}

TEST(LogLevel, toString)
{
    EXPECT_EQ("UNINITIALIZED", logLevelToString(LogLevel::UNINITIALIZED));
    EXPECT_EQ("NONE", logLevelToString(LogLevel::NONE));
    EXPECT_EQ("INFO", logLevelToString(LogLevel::INFO));
    EXPECT_EQ("WARN", logLevelToString(LogLevel::WARN));
    EXPECT_EQ("WARN", logLevelToString(LogLevel::WARNING));
    EXPECT_EQ("DEBUG", logLevelToString(LogLevel::DBG));
    EXPECT_EQ("ERROR", logLevelToString(LogLevel::ERROR));
    EXPECT_EQ("CRITICAL", logLevelToString(LogLevel::CRITICAL));
    EXPECT_EQ("DFATAL", logLevelToString(LogLevel::DFATAL));
    EXPECT_EQ("FATAL", logLevelToString(LogLevel::FATAL));
    EXPECT_EQ("FATAL", logLevelToString(LogLevel::MAX_LEVEL));
}

TEST(LogLevel, toStringAndBack)
{
    // Check that stringToLogLevel(logLevelToString()) is the identity function
    auto checkLevel = [](LogLevel level)
    {
        auto stringFrom = logLevelToString(level);
        auto outputLevel = stringToLogLevel(stringFrom);
        EXPECT_EQ(level, outputLevel)
            << "error converting " << level << " (" << static_cast<uint32_t>(level)
            << ") to string and back.  String is " << stringFrom;
    };

    // Check all of the named levels
    checkLevel(LogLevel::UNINITIALIZED);
    checkLevel(LogLevel::NONE);
    checkLevel(LogLevel::DBG);
    checkLevel(LogLevel::INFO);
    checkLevel(LogLevel::WARN);
    checkLevel(LogLevel::WARNING);
    checkLevel(LogLevel::ERROR);
    checkLevel(LogLevel::CRITICAL);
    checkLevel(LogLevel::DFATAL);
    checkLevel(LogLevel::FATAL);
}