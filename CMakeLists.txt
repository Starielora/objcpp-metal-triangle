cmake_minimum_required(VERSION 3.22)

project(objcpp-metal-triangle)

    add_subdirectory(deps)

    add_executable(objcpp-metal-triangle src/main.cpp assets/shaders.metal)

    set_source_files_properties(src/main.cpp PROPERTIES COMPILE_FLAGS -xobjective-c++)
    set_source_files_properties(assets/shaders.metal PROPERTIES LANGUAGE METAL)

    # TODO https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/platform_dependent_issues/HowToUseExistingOSXFrameworks
    target_link_libraries(objcpp-metal-triangle PRIVATE glfw "-framework Metal -framework QuartzCore")

    set_target_properties(objcpp-metal-triangle PROPERTIES XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES)
    target_compile_features(objcpp-metal-triangle PRIVATE cxx_std_17)
