#include "pch.h"
#include "graphics.res.h"

namespace res
{
    void register_graphics()
    {
        ff::data_reader assets_reader(::assets::graphics::data());
        ff::global_resources::add(assets_reader);
    }
}
