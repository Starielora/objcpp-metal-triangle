cmake_minimum_required(VERSION 3.22)

project(objcpp-metal-triangle)

    add_subdirectory(deps)

    add_executable(objcpp-metal-triangle src/main.cpp assets/shaders.metal)

    set_source_files_properties(src/main.cpp PROPERTIES COMPILE_FLAGS -xobjective-c++)

    # TODO https://gitlab.kitware.com/cmake/community/-/wikis/doc/cmake/platform_dependent_issues/HowToUseExistingOSXFrameworks
    target_link_libraries(objcpp-metal-triangle PRIVATE glfw Dear-ImGui "-framework Metal -framework QuartzCore")

    set_target_properties(objcpp-metal-triangle PROPERTIES CXX_STANDARD 17)
