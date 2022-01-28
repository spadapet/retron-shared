#include "pch.h"
#include "source/states/ui_view_state.h"

retron::ui_view_state::ui_view_state(std::shared_ptr<ff::ui_view> view)
    : ff::ui_view_state(view)
{
    view->size(ff::window_size{ retron::constants::RENDER_SIZE.cast<size_t>(), 1.0, DMDO_DEFAULT });
}
