set(CHIMI_SANDBOX_SHADER_SOURCES
    ${CMAKE_SOURCE_DIR}/shaders/mesh.vert
    ${CMAKE_SOURCE_DIR}/shaders/mesh.frag
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
        COMMENT "Compiling shader ${CHIMI_SHADER_NAME}"
        VERBATIM
    )

    list(APPEND CHIMI_SANDBOX_SHADER_OUTPUTS ${CHIMI_SHADER_OUTPUT})
endforeach()

add_custom_target(ChimiEngineShaders
    DEPENDS ${CHIMI_SANDBOX_SHADER_OUTPUTS}
)

add_executable(ChimiEngineSandbox
    src/main.cpp
)

add_dependencies(ChimiEngineSandbox ChimiEngineShaders)

target_link_libraries(ChimiEngineSandbox
    PRIVATE
        ChimiEngine
)

target_compile_features(ChimiEngineSandbox PRIVATE cxx_std_20)
