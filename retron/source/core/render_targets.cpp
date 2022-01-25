#include "pch.h"
#include "source/core/app_service.h"
#include "source/core/render_targets.h"

static const ff::point_size LOW_SIZE(retron::constants::RENDER_WIDTH, retron::constants::RENDER_HEIGHT);
static const ff::point_size HIGH_SIZE = ::LOW_SIZE * retron::constants::RENDER_SCALE;
static std::weak_ptr<ff::dxgi::depth_base> weak_depth;
static std::weak_ptr<ff::dxgi::texture_base> weak_texture_1080;
static std::weak_ptr<ff::dxgi::target_base> weak_target_1080;

static std::shared_ptr<ff::dxgi::depth_base> get_depth()
{
    std::shared_ptr<ff::dxgi::depth_base> depth = ::weak_depth.lock();
    if (!depth)
    {
        depth = ff::dxgi_client().create_depth(::LOW_SIZE, 0);
        ::weak_depth = depth;
    }

    return depth;
}

static std::shared_ptr<ff::dxgi::texture_base> get_texture_1080()
{
    std::shared_ptr<ff::dxgi::texture_base> texture = ::weak_texture_1080.lock();
    if (!texture)
    {
        texture = ff::dxgi_client().create_render_texture(::HIGH_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, &ff::dxgi::color_none());
        ::weak_texture_1080 = texture;
    }

    return texture;
}

static std::shared_ptr<ff::dxgi::target_base> get_target_1080()
{
    std::shared_ptr<ff::dxgi::target_base> target = ::weak_target_1080.lock();
    if (!target)
    {
        target = ff::dxgi_client().create_target_for_texture(::get_texture_1080());
        ::weak_target_1080 = target;
    }

    return target;
}

retron::render_targets::render_targets()
    : used_targets(retron::render_target_types::none)
    , viewport(::LOW_SIZE)
{}

void retron::render_targets::clear()
{
    if (ff::flags::has(this->used_targets, retron::render_target_types::palette_1) && this->target_palette_1)
    {
        this->target_palette_1->clear(ff::dxgi_client().frame_context(), ff::dxgi::color_none());
    }

    if (ff::flags::has(this->used_targets, retron::render_target_types::rgb_pma_2) && this->target_rgb_pma_1)
    {
        this->target_rgb_pma_1->clear(ff::dxgi_client().frame_context(), ff::dxgi::color_none());
    }

    this->used_targets = retron::render_target_types::none;
}

void retron::render_targets::render(ff::dxgi::target_base& target)
{
    ff::point_size target_logical_size = target.size().logical_pixel_size;
    bool direct_to_target = (target_logical_size == ::LOW_SIZE || target_logical_size == ::HIGH_SIZE);

    if (!direct_to_target)
    {
        if (!this->texture_1080)
        {
            this->texture_1080 = std::make_shared<ff::texture>(::get_texture_1080());
            this->target_1080 = ::get_target_1080();
        }

        this->target_1080->clear(ff::dxgi_client().frame_context(), ff::dxgi::color_none());
    }

    ff::dxgi::draw_ptr draw = direct_to_target
        ? ff::dxgi_client().global_draw_device().begin_draw(target, nullptr, ff::rect_fixed(0, 0, target_logical_size.x, target_logical_size.y), constants::RENDER_RECT)
        : ff::dxgi_client().global_draw_device().begin_draw(*this->target_1080, nullptr, constants::RENDER_RECT_HIGH, constants::RENDER_RECT);
    if (draw)
    {
        if (ff::flags::has(this->used_targets, retron::render_target_types::palette_1))
        {
            draw->push_palette(retron::app_service::get().palette());
            draw->draw_sprite(this->texture(retron::render_target_types::palette_1)->sprite_data(), ff::dxgi::transform::identity());
        }

        if (ff::flags::has(this->used_targets, retron::render_target_types::rgb_pma_2))
        {
            draw->push_pre_multiplied_alpha();
            draw->draw_sprite(this->texture(retron::render_target_types::rgb_pma_2)->sprite_data(), ff::dxgi::transform::identity());
        }
    }

    if (!direct_to_target)
    {
        ff::rect_fixed target_rect = this->viewport.view(target.size().logical_pixel_size).cast<ff::fixed_int>();
        draw = ff::dxgi_client().global_draw_device().begin_draw(target, nullptr, target_rect, constants::RENDER_RECT_HIGH);
        if (draw)
        {
            draw->push_sampler_linear_filter(true);
            draw->draw_sprite(this->texture_1080->sprite_data(), ff::dxgi::transform::identity());
        }
    }
}

const std::shared_ptr<ff::texture>& retron::render_targets::texture(retron::render_target_types target)
{
    this->used_targets = ff::flags::set(this->used_targets, target);

    switch (target)
    {
        case retron::render_target_types::palette_1:
            if (!this->texture_palette_1)
            {
                auto dxgi_texture = ff::dxgi_client().create_render_texture(::LOW_SIZE, DXGI_FORMAT_R8_UINT, 1, 1, 1, nullptr);
                this->texture_palette_1 = std::make_shared<ff::texture>(dxgi_texture);
            }
            return this->texture_palette_1;

        default:
        case retron::render_target_types::rgb_pma_2:
            if (!this->texture_rgb_pma_1)
            {
                auto dxgi_texture = ff::dxgi_client().create_render_texture(::LOW_SIZE, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 1, 1, nullptr);
                this->texture_rgb_pma_1 = std::make_shared<ff::texture>(dxgi_texture);
            }
            return this->texture_rgb_pma_1;
    }
}

const std::shared_ptr<ff::dxgi::target_base>& retron::render_targets::target(retron::render_target_types target)
{
    this->used_targets = ff::flags::set(this->used_targets, target);

    switch (target)
    {
        case retron::render_target_types::palette_1:
            if (!this->target_palette_1)
            {
                this->target_palette_1 = ff::dxgi_client().create_target_for_texture(this->texture(target)->dxgi_texture());
                this->target_palette_1->clear(ff::dxgi_client().frame_context(), ff::dxgi::color_none());
            }
            return this->target_palette_1;

        default:
        case retron::render_target_types::rgb_pma_2:
            if (!this->target_rgb_pma_1)
            {
                this->target_rgb_pma_1 = ff::dxgi_client().create_target_for_texture(this->texture(target)->dxgi_texture());
                this->target_rgb_pma_1->clear(ff::dxgi_client().frame_context(), ff::dxgi::color_none());
            }
            return this->target_rgb_pma_1;
    }
}

const std::shared_ptr<ff::dxgi::depth_base>& retron::render_targets::depth(retron::render_target_types target)
{
    if (!this->depth_)
    {
        this->depth_ = ::get_depth();
    }

    return this->depth_;
}
