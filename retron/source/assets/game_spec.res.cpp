#include "pch.h"
#include "game_spec.res.h"

namespace res
{
    void register_game_spec()
    {
        ff::data_reader assets_reader(::assets::game_spec::data());
        ff::global_resources::add(assets_reader);
    }
}
