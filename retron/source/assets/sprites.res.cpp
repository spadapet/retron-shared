#include "pch.h"
#include "sprites.res.h"

namespace res
{
    void register_sprites()
    {
        ff::data_reader assets_reader(::assets::sprites::data());
        ff::global_resources::add(assets_reader);
    }
}
