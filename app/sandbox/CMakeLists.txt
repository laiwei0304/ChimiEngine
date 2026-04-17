add_executable(ChimiEngineSandbox
    src/main.cpp
)

target_link_libraries(ChimiEngineSandbox
    PRIVATE
        ChimiEngine
)

target_compile_features(ChimiEngineSandbox PRIVATE cxx_std_20)
