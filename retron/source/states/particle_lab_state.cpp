#include "pch.h"
#include "source/core/app_service.h"
#include "source/states/particle_lab_state.h"
#include "source/ui/particle_lab_page.xaml.h"

retron::particle_lab_state::particle_lab_state(std::shared_ptr<ff::ui_view> view)
    : view(view)
{
    retron::particle_lab_page* page = Noesis::DynamicCast<retron::particle_lab_page*>(view->content());
    assert(page);

    if (page)
    {
        this->connections.emplace_front(page->clicked_sink().connect(std::bind(&retron::particle_lab_state::on_mouse_click, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4)));
    }
}

std::shared_ptr<ff::state> retron::particle_lab_state::advance_time()
{
    ff::scope_exit end_action = this->particles.advance_async();
    return nullptr;
}

void retron::particle_lab_state::render(ff::dxgi::command_context_base& context, ff::render_targets& targets)
{
    ff::dxgi::draw_ptr draw = retron::app_service::begin_palette_draw(context, targets);
    if (draw)
    {
        this->particles.render(*draw);
    }
}

void retron::particle_lab_state::on_mouse_click(int button, ff::point_float pos, std::string_view name, retron::particles::effect_t& effect)
{
    if (button == VK_LBUTTON)
    {
        effect.add(this->particles, pos.cast<ff::fixed_int>());
    }
}
