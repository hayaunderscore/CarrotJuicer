#pragma once

#include "imgui.h"
#include "spdlog/sinks/base_sink.h"
#include "fmt/core.h"

#include <mutex>

namespace gui
{
    class ImGuiSink : public spdlog::sinks::base_sink<std::mutex> 
    {
        public:
        void Draw();

        protected:
        ImGuiTextBuffer log;

        void sink_it_(const spdlog::details::log_msg& msg) override;
        void flush_() override;
    };

    using imgui_sink_mt = ImGuiSink;
}