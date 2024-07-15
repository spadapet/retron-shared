#include "pch.h"
#include "electrode.res.h"

namespace res
{
    void register_electrode()
    {
        ff::data_reader assets_reader(::assets::electrode::data());
        ff::global_resources::add(assets_reader);
    }
}
