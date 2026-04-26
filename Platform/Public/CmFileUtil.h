#pragma once

#include "CmEngine.h"

#ifndef CHIMI_RES_ROOT_DIR
#define CHIMI_RES_ROOT_DIR "Resource/"
#endif

#define CHIMI_RES_CONFIG_DIR CHIMI_RES_ROOT_DIR "Config/"
#define CHIMI_RES_SHADER_DIR CHIMI_RES_ROOT_DIR "Shader/"
#define CHIMI_RES_FONT_DIR CHIMI_RES_ROOT_DIR "Font/"
#define CHIMI_RES_MODEL_DIR CHIMI_RES_ROOT_DIR "Model/"
#define CHIMI_RES_MATERIAL_DIR CHIMI_RES_ROOT_DIR "Material/"
#define CHIMI_RES_TEXTURE_DIR CHIMI_RES_ROOT_DIR "Texture/"
#define CHIMI_RES_SCENE_DIR CHIMI_RES_ROOT_DIR "Scene/"

namespace chimi
{
inline std::string GetFileName(const std::string& filePath)
{
    if (filePath.empty())
    {
        return filePath;
    }

    return std::filesystem::path(filePath).filename().string();
}

inline void FormatFileSize(std::uintmax_t fileSize, float* outSize, std::string& outUnit)
{
    float size = static_cast<float>(fileSize);
    if (size < 1024.0f)
    {
        outUnit = "B";
    }
    else if (size < 1024.0f * 1024.0f)
    {
        size /= 1024.0f;
        outUnit = "KB";
    }
    else if (size < 1024.0f * 1024.0f * 1024.0f)
    {
        size /= (1024.0f * 1024.0f);
        outUnit = "MB";
    }
    else
    {
        size /= (1024.0f * 1024.0f * 1024.0f);
        outUnit = "GB";
    }

    *outSize = size;
}

inline std::string FormatSystemTime(std::filesystem::file_time_type)
{
    const std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    const std::tm* tm = std::localtime(&time);

    std::stringstream ss;
    ss << std::put_time(tm, "%Y/%m/%d %H:%M");
    return ss.str();
}

inline std::vector<char> ReadCharArrayFromFile(const std::string& filePath)
{
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Could not open the file: " + filePath);
    }

    const size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
    file.close();
    return buffer;
}
}
