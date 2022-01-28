#include "pch.h"
#include "source/core/app_service.h"
#include "source/states/transition_state.h"

retron::transition_state::transition_state(std::shared_ptr<ff::state> old_state, std::shared_ptr<ff::state> new_state, std::string_view image_resource, size_t speed, size_t vertical_pixel_stop)
    : temp_target(retron::constants::RENDER_SIZE.cast<size_t>())
    , old_state(old_state)
    , new_state(new_state)
    , image(image_resource)
    , counter(0)
    , speed(static_cast<int>(speed))
    , offset_stop(static_cast<int>(vertical_pixel_stop))
{
    assert(this->new_state);

    ff::point_size size = constants::RENDER_SIZE.cast<size_t>();

    auto dxgi_texture = ff::dxgi_client().create_render_texture(size, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, &ff::dxgi::color_black());
    auto dxgi_texture2 = ff::dxgi_client().create_render_texture(size, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, &ff::dxgi::color_black());

    this->texture = std::make_shared<ff::texture>(dxgi_texture);
    this->texture2 = std::make_shared<ff::texture>(dxgi_texture2);
    this->target = ff::dxgi_client().create_target_for_texture(this->texture->dxgi_texture());
    this->target2 = ff::dxgi_client().create_target_for_texture(this->texture2->dxgi_texture());
}

std::shared_ptr<ff::state> retron::transition_state::advance_time()
{
    if (this->old_state)
    {
        this->counter += 40;

        if (this->counter > constants::RENDER_WIDTH)
        {
            this->old_state = nullptr;
            this->counter = 0;
        }
    }
    else
    {
        this->counter += this->speed;

        if (this->counter >= constants::RENDER_HEIGHT - this->offset_stop)
        {
            this->new_state->advance_time();
            return this->new_state;
        }
    }

    return nullptr;
}

void retron::transition_state::render(ff::dxgi::command_context_base& context, ff::render_targets& targets)
{
    this->target->clear(context, ff::dxgi::color_black());
    this->target2->clear(context, ff::dxgi::color_black());

    ff::fixed_int half_height = constants::RENDER_HEIGHT / 2;
    retron::app_service& app = retron::app_service::get();

    if (this->old_state)
    {
        targets.push(this->temp_target);
        this->old_state->render(context, targets);
        targets.pop(context, this->target.get(), app.palette());

        ff::rect_fixed rect = constants::RENDER_RECT.right_edge();
        rect.left -= std::min(this->counter, constants::RENDER_WIDTH);

        ff::dxgi::draw_ptr draw = ff::dxgi_client().global_draw_device().begin_draw(context, *this->target, nullptr, rect, rect);
        if (draw)
        {
            draw->draw_filled_rectangle(rect.cast<float>(), ff::dxgi::color_black());
        }
    }
    else if (this->counter <= half_height)
    {
        ff::fixed_int offset = half_height - std::min(half_height, this->counter);
        ff::rect_fixed rect(offset, offset, constants::RENDER_WIDTH - offset, constants::RENDER_HEIGHT - offset);
        ff::dxgi::draw_ptr draw = ff::dxgi_client().global_draw_device().begin_draw(context, *this->target, nullptr, rect, rect);
        if (draw)
        {
            draw->push_palette(app.palette());
            draw->draw_sprite(this->image->sprite_data(), ff::dxgi::transform::identity());
        }
    }
    else
    {
        // Draw gradient image
        {
            ff::dxgi::draw_ptr draw = ff::dxgi_client().global_draw_device().begin_draw(context, *this->target, nullptr, constants::RENDER_RECT, constants::RENDER_RECT);
            if (draw)
            {
                draw->push_palette(app.palette());
                draw->draw_sprite(this->image->sprite_data(), ff::dxgi::transform::identity());
            }
        }

        targets.push(this->temp_target);
        this->new_state->render(context, targets);
        targets.pop(context, this->target2.get(), app.palette());

        // Draw new state
        {
            ff::fixed_int offset = half_height - std::min(half_height, this->counter - half_height);
            ff::rect_fixed rect(offset, offset, constants::RENDER_WIDTH - offset, constants::RENDER_HEIGHT - offset);
            ff::dxgi::draw_ptr draw = ff::dxgi_client().global_draw_device().begin_draw(context, *this->target, nullptr, rect, rect);
            if (draw)
            {
                draw->draw_sprite(this->texture2->sprite_data(), ff::dxgi::transform::identity());
            }
        }
    }

    ff::dxgi::draw_ptr draw = ff::dxgi_client().global_draw_device().begin_draw(context,
        targets.target(context, ff::render_target_type::rgba_pma), &targets.depth(context),
        constants::RENDER_RECT, constants::RENDER_RECT);

    if (draw)
    {
        draw->draw_sprite(this->texture->sprite_data(), ff::dxgi::transform::identity());
    }
}
