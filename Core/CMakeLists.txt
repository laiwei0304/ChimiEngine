add_library(chimi_core
        Private/CmApplication.cpp
        Private/Render/CmRenderContext.cpp
        Private/Render/CmRenderTarget.cpp
        Private/Render/CmRenderer.cpp
        Private/Render/CmMesh.cpp
        Private/Render/CmTexture.cpp
        Private/Render/CmSampler.cpp
        Private/Render/CmMaterial.cpp

        Private/ECS/CmUUID.cpp
        Private/ECS/CmNode.cpp
        Private/ECS/CmScene.cpp
        Private/ECS/CmEntity.cpp
        Private/ECS/Component/CmLookAtCameraComponent.cpp
        Private/ECS/System/CmMaterialSystem.cpp
        Private/ECS/System/CmBaseMaterialSystem.cpp
        Private/ECS/System/CmUnlitMaterialSystem.cpp
)

target_include_directories(chimi_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/Public
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(chimi_core
    PUBLIC
        chimi_platform
        chimi_entt
)

target_compile_features(chimi_core PUBLIC cxx_std_20)
