set(CHIMI_00_FIRST_TRIANGLE_SHADER_SOURCES
    ${CMAKE_SOURCE_DIR}/Resource/Shader/00_hello_triangle.vert
    ${CMAKE_SOURCE_DIR}/Resource/Shader/00_hello_triangle.frag
)

set(CHIMI_00_FIRST_TRIANGLE_SHADER_OUTPUTS)

foreach(CHIMI_SHADER_SOURCE IN LISTS CHIMI_00_FIRST_TRIANGLE_SHADER_SOURCES)
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

    list(APPEND CHIMI_00_FIRST_TRIANGLE_SHADER_OUTPUTS ${CHIMI_SHADER_OUTPUT})
endforeach()

add_custom_target(chimi_00_first_triangle_shaders
    DEPENDS ${CHIMI_00_FIRST_TRIANGLE_SHADER_OUTPUTS}
)

add_executable(00_FirstTriangle Main.cpp)
add_dependencies(00_FirstTriangle chimi_00_first_triangle_shaders)
target_link_libraries(00_FirstTriangle PRIVATE chimi_core)
