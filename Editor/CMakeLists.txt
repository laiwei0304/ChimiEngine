add_library(chimi_editor
        Private/CmEditorApp.cpp
        Private/CmEditorTitleBar.cpp
        Private/CmImGuiLayer.cpp
        Private/CmEditorLayer.cpp
        Private/CmEditorContext.cpp
        Private/Panel/CmPanel.cpp)

target_include_directories(chimi_editor
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(chimi_editor
        PUBLIC
        chimi_core
        PRIVATE
        imgui
)

target_compile_features(chimi_editor PUBLIC cxx_std_20)

set(CHIMI_EDITOR_SHADER_SOURCES
        ${CMAKE_SOURCE_DIR}/Resource/Shader/03_unlit_material.vert
        ${CMAKE_SOURCE_DIR}/Resource/Shader/03_unlit_material.frag
)

set(CHIMI_EDITOR_SHADER_OUTPUTS)

foreach(CHIMI_SHADER_SOURCE IN LISTS CHIMI_EDITOR_SHADER_SOURCES)
    get_filename_component(CHIMI_SHADER_NAME ${CHIMI_SHADER_SOURCE} NAME)
    set(CHIMI_SHADER_OUTPUT ${CHIMI_SHADER_OUTPUT_DIR}/${CHIMI_SHADER_NAME}.spv)

    add_custom_command(
            OUTPUT ${CHIMI_SHADER_OUTPUT}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CHIMI_SHADER_OUTPUT_DIR}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${CHIMI_SHADER_SOURCE} -o ${CHIMI_SHADER_OUTPUT}
            DEPENDS ${CHIMI_SHADER_SOURCE}
            COMMENT "Compiling editor shader ${CHIMI_SHADER_NAME}"
            VERBATIM
    )

    list(APPEND CHIMI_EDITOR_SHADER_OUTPUTS ${CHIMI_SHADER_OUTPUT})
endforeach()

add_custom_target(chimi_editor_shaders
        DEPENDS ${CHIMI_EDITOR_SHADER_OUTPUTS}
)

add_executable(ChimiEditor
        Private/CmEditorEntry.cpp)

add_dependencies(ChimiEditor chimi_editor_shaders)

target_link_libraries(ChimiEditor
        PRIVATE
        chimi_editor
)
