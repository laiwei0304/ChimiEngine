#pragma once

#include "CmEngine.h"

#include "spdlog/common.h"

namespace chimi{
    class CmLog{
    public:
        CmLog(const CmLog&) = delete;
        CmLog &operator=(const CmLog&) = delete;
        static void Init();

        static CmLog* GetLoggerInstance(){
            return &sLoggerInstance;
        }

        template<typename... Args>
        void Log(spdlog::source_loc loc, spdlog::level::level_enum lvl, spdlog::format_string_t<Args...> fmt, Args &&...args){
            spdlog::memory_buf_t buf;
            fmt::vformat_to(fmt::appender(buf), fmt, fmt::make_format_args(args...));
            Log(loc, lvl, buf);
        }
    private:
        CmLog() = default;

        void Log(spdlog::source_loc loc, spdlog::level::level_enum lvl, const spdlog::memory_buf_t &buffer);

        static CmLog sLoggerInstance;
    };
#define CHIMI_LOG_LOGGER_CALL(cmLog, level, ...)\
(cmLog)->Log(spdlog::source_loc{__FILE__, __LINE__, SPDLOG_FUNCTION}, level, __VA_ARGS__)

#define LOG_T(...) CHIMI_LOG_LOGGER_CALL(chimi::CmLog::GetLoggerInstance(), spdlog::level::trace, __VA_ARGS__)
#define LOG_D(...) CHIMI_LOG_LOGGER_CALL(chimi::CmLog::GetLoggerInstance(), spdlog::level::debug, __VA_ARGS__)
#define LOG_I(...) CHIMI_LOG_LOGGER_CALL(chimi::CmLog::GetLoggerInstance(), spdlog::level::info, __VA_ARGS__)
#define LOG_W(...) CHIMI_LOG_LOGGER_CALL(chimi::CmLog::GetLoggerInstance(), spdlog::level::warn, __VA_ARGS__)
#define LOG_E(...) CHIMI_LOG_LOGGER_CALL(chimi::CmLog::GetLoggerInstance(), spdlog::level::err, __VA_ARGS__)
}
