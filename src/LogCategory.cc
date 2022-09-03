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

#include "LogCategory.h"

#include <cstdio>
#include <cstdlib>

namespace tinylog
{
    LogCategory::LogCategory(LoggerDB *db)
        : effectiveLevel_{LogLevel::ERROR},
          level_{static_cast<uint32_t>(LogLevel::ERROR)},
          parent_{nullptr},
          name_{},
          db_{} {}

    LogCategory::LogCategory(StringPiece name, LogCategory *parent)
        : effectiveLevel_{parent->getEffectiveLevel()},
          level_{static_cast<uint32_t>(LogLevel::MAX_LEVEL) | FLAG_INHERIT},
          parent_{parent},
          name_{LogName::canonicalize(name)},
          db_{parent->getDB()},
          nextSibling_{parent_->firstChild_}
    {
        parent_->firstChild_ = this;
    }

} // namespace tinylog
