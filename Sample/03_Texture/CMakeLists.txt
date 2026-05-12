set(CHIMI_03_TEXTURE_SHADER_SOURCES
        ${CMAKE_SOURCE_DIR}/Resource/Shader/02_descriptor_set.vert
        ${CMAKE_SOURCE_DIR}/Resource/Shader/02_descriptor_set.frag
)

set(CHIMI_03_TEXTURE_SHADER_OUTPUTS)

foreach(CHIMI_SHADER_SOURCE IN LISTS CHIMI_03_TEXTURE_SHADER_SOURCES)
    get_filename_component(CHIMI_SHADER_NAME ${CHIMI_SHADER_SOURCE} NAME)
    set(CHIMI_SHADER_OUTPUT ${CHIMI_SHADER_OUTPUT_DIR}/${CHIMI_SHADER_NAME}.spv)

    add_custom_command(
            OUTPUT ${CHIMI_SHADER_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CHIMI_SHADER_OUTPUT_DIR}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${CHIMI_SHADER_SOURCE} -o ${CHIMI_SHADER_OUTPUT}
            DEPENDS ${CHIMI_SHADER_SOURCE}
            COMMENT "Compiling shader ${CHIMI_SHADER_NAME}"
            VERBATIM
    )

    list(APPEND CHIMI_03_TEXTURE_SHADER_OUTPUTS ${CHIMI_SHADER_OUTPUT})
endforeach()

add_custom_target(chimi_03_texture_shaders
        DEPENDS ${CHIMI_03_TEXTURE_SHADER_OUTPUTS}
)

add_executable(03_Texture Main.cpp)
add_dependencies(03_Texture chimi_03_texture_shaders)
target_link_libraries(03_Texture PRIVATE chimi_core)
