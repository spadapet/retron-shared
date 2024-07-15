#include "pch.h"
#include "bonus.res.h"

namespace res
{
    void register_bonus()
    {
        ff::data_reader assets_reader(::assets::bonus::data());
        ff::global_resources::add(assets_reader);
    }
}
