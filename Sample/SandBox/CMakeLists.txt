set(CHIMI_SANDBOX_SHADER_SOURCES
    ${CMAKE_SOURCE_DIR}/Resource/Shader/01_hello_buffer.vert
    ${CMAKE_SOURCE_DIR}/Resource/Shader/01_hello_buffer.frag
    ${CMAKE_SOURCE_DIR}/Resource/Shader/03_unlit_material.vert
    ${CMAKE_SOURCE_DIR}/Resource/Shader/03_unlit_material.frag
)

set(CHIMI_SANDBOX_SHADER_OUTPUTS)

foreach(CHIMI_SHADER_SOURCE IN LISTS CHIMI_SANDBOX_SHADER_SOURCES)
    get_filename_component(CHIMI_SHADER_NAME ${CHIMI_SHADER_SOURCE} NAME)
    set(CHIMI_SHADER_OUTPUT ${CHIMI_SHADER_OUTPUT_DIR}/${CHIMI_SHADER_NAME}.spv)

    add_custom_command(
        OUTPUT ${CHIMI_SHADER_OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CHIMI_SHADER_OUTPUT_DIR}
        COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${CHIMI_SHADER_SOURCE} -o ${CHIMI_SHADER_OUTPUT}
        DEPENDS ${CHIMI_SHADER_SOURCE}
        COMMENT "Compiling sandbox shader ${CHIMI_SHADER_NAME}"
        VERBATIM
    )

    list(APPEND CHIMI_SANDBOX_SHADER_OUTPUTS ${CHIMI_SHADER_OUTPUT})
endforeach()

add_custom_target(chimi_sandbox_shaders
    DEPENDS ${CHIMI_SANDBOX_SHADER_OUTPUTS}
)

add_executable(SandBox
    Main.cpp
    CmEventTesting.cpp
)

add_dependencies(SandBox chimi_sandbox_shaders)

target_link_libraries(SandBox
    PRIVATE
        chimi_core
)
