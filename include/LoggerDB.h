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

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <functional>
#include <mutex>

#include "StringPiece.h"
#include "LogName.h"

namespace tinylog
{
    class LogCategory;
    class LogConfig;
    class LogHandler;
    class LogHandlerFactory;

    enum class LogLevel : uint32_t;

    /**
     * LoggerDB stores the set of LogCategory objects.
     */
    class LoggerDB
    {
        using ContextCallback = std::function<std::string() const>;

    public:
        /**
         * Get the main LoggerDB singleton.
         */
        static LoggerDB &get();

        ~LoggerDB();

        /**
         * Get the LogCategory for the specified name.
         *
         * This creates the LogCategory for the specified name if it does not exist
         * already.
         */
        LogCategory *getCategory(tinylog::StringPiece name);

        /**
         * Get the LogCategory for the specified name, if it already exists.
         *
         * This returns nullptr if no LogCategory has been created yet for the
         * specified name.
         */
        LogCategory *getCategoryOrNull(tinylog::StringPiece name);

        /**
         * Set the log level for the specified category.
         *
         * Message logged to a specific log category will be ignored unless the
         * message log level is greater than the LogCategory's effective log level.
         *
         * If inherit is true, LogCategory's effective log level is the minimum of
         * its level and it's parent category's effective log level. If inherit is
         * false, the LogCategory's effective log level is simply its log level.
         * (Setting inherit to false is necessary if you want a child LogCategory to
         * use a less verbose level than its parent categories.)
         */
        void setLevel(tinylog::StringPiece name, LogLevel level, bool inherit = true);
        void setLevel(LogCategory *category, LogLevel level, bool inherit = true);

        /**
         * Get a LogConfig object describing the current state of the LoggerDB.
         */
        LogConfig getConfig() const;

        /**
         * Get a LogConfig object fully describing the state of the LoggerDB.
         *
         * This is similar to getConfig(), but it returns LogCategoryConfig objects
         * for all defined log categories, including ones that are using the default
         * configuration setting.
         */
        LogConfig getFullConfig() const;

        /**
         * Update the current LoggerDB state with the specified LogConfig settings.
         *
         * Log categories and handlers listed in the LogConfig object will be updated
         * to the new state listed in the LogConfig. Settings on categories and
         * handlers not listed in the config will be left as-is.
         */
        void updateConfig(const LogConfig &config);

        /**
         * Reset the current LoggerDB state to the specified LogConfig settings.
         *
         * All logCategories not mentioned in the new LogConfig will have all
         * currently configured log handlers removed and their log level set to its
         * default state. For the root category the default log level is
         * kDefaultLogLevel (see logLevel.h); for all other categories the default
         * level is MAX_LEVEL with log level inheritance enabled.
         *
         * LogCategories listed in the new config but without LogHandler information
         * defined will have all existing handlers removed.
         */
        void resetConfig(const LogConfig &config);

        /**
         * Remove all registered LogHandlers on all LogCategory objects.
         *
         * This is called on the main LoggerDB object during shutdown.
         */
        void cleanupHandlers();

        /**
         * Call flush() on all LogHandler objects registered on any LogCategory in
         * this LoggerDB.
         *
         * Returns the number of registered logHandlers.
         */
        size_t flushAllHandlers();

        /**
         * Register a LogHandlerFactory.
         *
         * The LogHandlerFactory will be used to create LogHandler objects from a
         * LogConfig object during updateConfig() and resetConfig() calls.
         *
         * Only one factory can be registered for a given handler type name.
         * LogHandlerFactory::getType() returns the handler type supported by this
         * LogHandlerFactory.
         *
         * If an existing LogHandlerFactory is already registered with this type name
         * and replaceExisting is false a std::range_error will be thrown.
         * Otherwise, if replaceExisting is true, the new factory will replace the
         * existing factory.
         */
        void registerHandlerFactory(
            std::unique_ptr<LogHandlerFactory> factory, bool replaceExisting = false);

        /**
         * Remove a registered LogHandlerFactory.
         *
         * The type parameter should be the name of the handler type, as returned by
         * LogHandlerFactory::getType().
         *
         * Throws std::range_error if no handler factory with this type name exists.
         */
        void unregisterHandlerFactory(tinylog::StringPiece type);

        /**
         * Initialize the logCategory* and std::atomic<LogLevel> used by an XLOG()
         * statement.
         *
         * Returns the current effective LogLevel of the category.
         */
        LogLevel xlogInit(
            tinylog::StringPiece categoryName,
            std::atomic<LogLevel> *xlogCategoryLevel,
            LogCategory **xlogCategory);
        LogLevel xlogInitCategory(
            tinylog::StringPiece categoryName,
            LogCategory **xlogCategory,
            std::atomic<bool> *isInitialized);

        enum TestConstructorArg
        {
            TESTING
        };

        /**
         * Construct a loggerDB for testing purrpose.
         *
         * Most callers should not need this function, and should use
         * LoggerDB::get() to obtain the main LoggerDB singleton. This function
         * exists mainly to allow testing LoggerDB objects in unit tests.
         * It requires an explicit argument just to prevent callers from calling it
         * unintentionally.
         */
        explicit LoggerDB(TestConstructorArg);

        /**
         * Add a new context string callback to the list.
         *
         * The callbacks will be invoked during the construction of log messges,
         * and returned strings will be appended in order to the tail of log
         * log entry prefixes with space prepended to each item.
         */
        void addContextCallback(ContextCallback);

        /**
         * Return a context string to be appended after default log prefixes.
         *
         * The context string is cutomized through adding context callbacks to
         * LoggerDB objects.
         */
        std::string getContextString() const;

        /**
         * internalWarning() is used to report a problem when something goes wrong
         * internally in the logging library.
         *
         * We can't log these message through the normal logging flow since logging
         * itself has failed.
         *
         * Example scenarios where this is used:
         * - We fail to write to a log file (for instance, when the disk is full)
         * - A LogHandler throws an unexpected exception
         */
        template <typename... Args>
        static void internalWarning(
            tinylog::StringPiece file, int lineNumber, Args &&...args) noexcept
        {
            internalWarningImpl(
                file, lineNumber, std::string &);
        }

        using InternalWarningHandler =
            void (*)(tinylog::StringPiece file, int lineNumber, std::string &&);

        /**
         * Set a function to be called when the logging library generates an internal
         * warning.
         *
         * The supplied handler should never throw exceptions.
         *
         * If a null handler is supplied, the default built-in handler will be used.
         *
         * The default handler reports the message with _CrtDbgReport(_CRT_WARN) on
         * Windows, and prints the message to stderr on other platforms.  It also
         * rate limits messages if they are arriving too quickly.
         */
        static void setInternalWarningHandler(InternalWarningHandler handler);

    private:
        using LoggerNameMap = std::unordered_map<
            tinylog::StringPiece,
            std::unique_ptr<LogCategory>,
            LogName::Hash,
            LogName::Equals>;

        using HandlerFactoryMap =
            std::unordered_map<std::string, std::unique_ptr<LogHandlerFactory>>;
        using HandlerMap = std::unordered_map<std::string, std::weak_ptr<LogHandler>>;
        struct HandlerInfo
        {
            HandlerFactoryMap factories;
            HandlerMap handlers;
        };

        class ContextCallbackList
        {
        public:
            void addCallback(ContextCallback);
            std::string getContextString() const;
            ~ContextCallbackList();

        private:
            class CallbacksObj;
            std::atomic<CallbacksObj *> callbacks_(nullptr);
            std::mutex writeMutex_;
        };

        // Forbidden copy constructor and assignment operator
        LoggerDB(LoggerDB const &) = delete;
        LoggerDB &operator=(LoggerDB const &) = delete;

        LoggerDB();
        LogCategory *getOrCreateCategoryLocked(
            LoggerNameMap &loggersByName, tinylog::StringPiece name);
        LogCategory *createCategoryLocked(
            LoggerNameMap &loggersByName,
            tinylog::StringPiece name,
            LogCategory *parent);

        using NewHandlerMap =
            std::unordered_map<std::string, std::shared_ptr<LogHandler>>;
        using OldToNewHandlerMap = std::
            unordered_map<std::shared_ptr<LogHandler>, std::shared_ptr<LogHandler>>;
        LogConfig getConfigImpl(bool includeAllCategories) const;
        void startConfigUpdate(
            const Synchronized<HandlerInfo>::LockedPtr &handlerInfo,
            const LogConfig &config,
            NewHandlerMap *handlers,
            OldToNewHandlerMap *oldToNewHandlerMap);
        void finishConfigUpdate(
            const Synchronized<HandlerInfo>::LockedPtr &handlerInfo,
            NewHandlerMap *handlers,
            OldToNewHandlerMap *oldToNewHandlerMap);
        std::vector<std::shared_ptr<LogHandler>> buildCategoryHandlerList(
            const NewHandlerMap &handlerMap,
            tinylog::StringPiece categoryName,
            const std::vector<std::string> &categoryHandlerNames);

        static void internalWarningImpl(
            tinylog::StringPiece filename, int lineNumber, std::string &&msg) noexcept;
        static void defaultInternalWarningImpl(
            tinylog::StringPiece filename, int lineNumber, std::string &&msg) noexcept;
        
        /**
         * A map of LogCategory objects by name.
         * 
         * Lookups can be performed using arbitrary StringPiece values that do not
         * have to be in canonical form.
         */
        tinylog::Synchronized<LoggerNameMap> loggersByName_;

        /**
         * The LogHandlers and LogHandlerFactories.
         * 
         * For lock ordering purposes, if you need to acquire both the loggersByName_
         * and handlerInfo_ locks, the handlerInfo_ lock must be acquired first.
         */
        tinylog::Synchronized<HandlerInfo> handlerInfo_;

        /**
         * Callbacks returning context strings.
         * 
         * Exceptions from the callbacks are catched and reflected in corresponding
         * position in log entries
         */
        ContextCallbackList caontextCallbacks_;
        static std::atomic<InternalWarningHandler> warningHandler_;
    };

} // namespace tinylog
