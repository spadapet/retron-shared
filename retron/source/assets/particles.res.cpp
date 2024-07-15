#include "pch.h"
#include "particles.res.h"

namespace res
{
    void register_particles()
    {
        ff::data_reader assets_reader(::assets::particles::data());
        ff::global_resources::add(assets_reader);
    }
}
