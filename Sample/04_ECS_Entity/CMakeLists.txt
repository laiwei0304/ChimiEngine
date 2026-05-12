set(CHIMI_04_ECS_ENTITY_SHADER_SOURCES
        ${CMAKE_SOURCE_DIR}/Resource/Shader/01_hello_buffer.vert
        ${CMAKE_SOURCE_DIR}/Resource/Shader/01_hello_buffer.frag
)

set(CHIMI_04_ECS_ENTITY_SHADER_OUTPUTS)

foreach(CHIMI_SHADER_SOURCE IN LISTS CHIMI_04_ECS_ENTITY_SHADER_SOURCES)
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

    list(APPEND CHIMI_04_ECS_ENTITY_SHADER_OUTPUTS ${CHIMI_SHADER_OUTPUT})
endforeach()

add_custom_target(chimi_04_ecs_entity_shaders
    DEPENDS ${CHIMI_04_ECS_ENTITY_SHADER_OUTPUTS}
)

add_executable(04_ECS_Entity Main.cpp)
add_dependencies(04_ECS_Entity chimi_04_ecs_entity_shaders)
target_link_libraries(04_ECS_Entity PRIVATE chimi_core)
