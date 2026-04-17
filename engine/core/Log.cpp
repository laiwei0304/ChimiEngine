#include "core/Log.h"

#include <memory>

#include "spdlog/pattern_formatter.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"

namespace chimi::core
{
void InitializeLogging()
{
    static bool initialized = false;
    if (initialized)
    {
        return;
    }

    auto logger = spdlog::stdout_color_mt("chimi");
    logger->set_pattern("[%H:%M:%S] [%^%l%$] %v");
    spdlog::set_default_logger(std::move(logger));
    spdlog::set_level(spdlog::level::trace);
    spdlog::flush_on(spdlog::level::err);

    initialized = true;
}
}
