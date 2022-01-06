cmake_minimum_required(VERSION 3.22)

# glfw
add_subdirectory(glfw)

# imgui
add_library(Dear-ImGui STATIC
    imgui/imconfig.h
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_internal.h
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imstb_rectpack.h
    imgui/imstb_textedit.h
    imgui/imstb_truetype.h

    # backends
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_glfw.h
    imgui/backends/imgui_impl_opengl3_loader.h
    imgui/backends/imgui_impl_opengl3.cpp
    imgui/backends/imgui_impl_opengl3.h
    imgui/backends/imgui_impl_metal.h
    imgui/backends/imgui_impl_metal.mm
    )

target_include_directories(Dear-ImGui PUBLIC imgui imgui/backends)
target_link_libraries(Dear-ImGui PRIVATE glfw)
target_compile_options(Dear-ImGui PRIVATE -fobjc-arc)
target_compile_definitions(Dear-ImGui PUBLIC __OBJC__)
