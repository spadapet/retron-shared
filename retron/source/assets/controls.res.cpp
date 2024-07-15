#include "pch.h"
#include "controls.res.h"

namespace res
{
    void register_controls()
    {
        ff::data_reader assets_reader(::assets::controls::data());
        ff::global_resources::add(assets_reader);
    }
}
