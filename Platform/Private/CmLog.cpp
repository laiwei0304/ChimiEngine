#include "CmLog.h"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace chimi{

    CmLog CmLog::sLoggerInstance{};
    static std::shared_ptr<spdlog::logger> sSpdLogger{};

    void CmLog::Init() {
        sSpdLogger = spdlog::stdout_color_mt("logger");
        sSpdLogger->set_level(spdlog::level::trace);
        sSpdLogger->set_pattern("%^%H:%M:%S:%e [%P-%t] [%1!L] [%20s:%-4#] - %v%$");
        sSpdLogger->flush_on(spdlog::level::trace);
    }

    void CmLog::Log(spdlog::source_loc loc, spdlog::level::level_enum lvl, const spdlog::memory_buf_t &buffer) {
        sSpdLogger->log(loc, lvl, spdlog::string_view_t(buffer.data(), buffer.size()));
    }
}
