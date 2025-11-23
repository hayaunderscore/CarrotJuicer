target_sources(
    CarrotJuicer
    PUBLIC

    config.cpp
    dllmain.cpp
    edb.cpp
    hook.cpp
    mdb.cpp
    notifier.cpp
    # proxy.cpp -- not needed, we're not compiling this as version.dll anymore
    requests.cpp
    responses.cpp
)

target_include_directories(
    CarrotJuicer
    PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
)
