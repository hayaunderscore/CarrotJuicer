#pragma once

#include "aetherim/wrapper.hpp"

namespace gallop 
{
    extern std::unique_ptr<Wrapper> wrapper;
    extern Image *umaimg;
    
    void init();
}