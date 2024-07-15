#include "pch.h"
#include "player.res.h"

namespace res
{
    void register_player()
    {
        ff::data_reader assets_reader(::assets::player::data());
        ff::global_resources::add(assets_reader);
    }
}
