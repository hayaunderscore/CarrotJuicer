target_sources(
    CarrotJuicer
    PUBLIC

    guiwindow.cpp
    # ImGui backend implementations
    imgui_impl_opengl3.cpp # Makes DXVK lag a lot more if I use DX11...
    imgui_impl_win32.cpp
    # ImGui base implementation
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    # Log sink for spdlog
    imgui_sink.cpp
)

target_include_directories(
    CarrotJuicer
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
