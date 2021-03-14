#include "pch.h"
#include "source/level/particles.h"

retron::particles::particles()
    : async_event(ff::create_event())
{
    this->particles_new.reserve(256);
    this->particles_async.reserve(512);
    this->groups.reserve(64);
}

ff::end_scope_action retron::particles::advance_async()
{
    ff::thread_pool::get()->add_task(std::bind(&particles::advance_now, this));
    return std::bind(&particles::advance_block, this);
}

void retron::particles::advance_block()
{
    ff::wait_for_event_and_reset(this->async_event);

    if (this->particles_new.size())
    {
        this->particles_async.insert(this->particles_async.end(), this->particles_new.begin(), this->particles_new.end());
        this->particles_new.clear();
    }

    for (auto& i : this->groups)
    {
        if (!i.refs)
        {
            // Allow reuse of this group
            i.refs = -1;
            i.effect_id = 0;
            i.animations.clear();
        }
    }
}

void retron::particles::advance_now()
{
    for (size_t i = 0; i < this->particles_async.size(); )
    {
        retron::particles::particle_t& p = particles_async[i];
        if (p.delay > 0)
        {
            p.delay--;
            i++;
        }
        else if (p.life > 1)
        {
            p.life--;
            p.angle += p.angle_vel;
            p.dist += p.dist_vel;
            p.spin += p.spin_vel;
            p.timer += ff::constants::seconds_per_advance_f;
            i++;
        }
        else
        {
            this->release_group(p.group);
            p = std::move(this->particles_async.back());
            this->particles_async.pop_back();
        }
    }

    ::SetEvent(this->async_event);
}

unsigned short retron::particles::add_group(const ff::pixel_transform& transform, int effect_id, int count, const std::vector<std::shared_ptr<ff::animation_base>>& animations)
{
    retron::particles::group_t group;
    DirectX::XMStoreFloat4x4(&group.matrix, transform.matrix());
    group.transform = transform;
    group.refs = count;
    group.effect_id = effect_id;
    group.animations = animations;

    for (size_t i = 0; i < this->groups.size(); i++)
    {
        if (this->groups[i].refs == -1)
        {
            this->groups[i] = std::move(group);
            return static_cast<unsigned short>(i);
        }
    }

    this->groups.push_back(std::move(group));
    return static_cast<unsigned short>(this->groups.size() - 1);
}

void retron::particles::release_group(unsigned short group_id)
{
    --this->groups[group_id].refs;
}

const DirectX::XMFLOAT4X4& retron::particles::matrix(unsigned short group_id) const
{
    assert(this->groups[group_id].refs > 0);
    return this->groups[group_id].matrix;
}

void retron::particles::render(ff::draw_base& draw, uint8_t type)
{
    ff::transform transform = ff::transform::identity();

    for (const retron::particles::particle_t& p : this->particles_async)
    {
        if (!p.delay && type == p.type)
        {
            DirectX::XMFLOAT2 angle_cos_sin;
            DirectX::XMScalarSinCosEst(&angle_cos_sin.y, &angle_cos_sin.x, p.angle);
            DirectX::XMFLOAT2 pos(angle_cos_sin.x * p.dist, angle_cos_sin.y * p.dist);
            DirectX::XMStoreFloat2(&pos,
                DirectX::XMVector2Transform(
                    DirectX::XMLoadFloat2(&pos),
                    DirectX::XMLoadFloat4x4(&matrix(p.group))));

            if (p.is_color())
            {
                draw.draw_palette_filled_rectangle(ff::rect_float(pos.x - p.size, pos.y - p.size, pos.x + p.size, pos.y + p.size), p.color_);
            }
            else
            {
                transform.position = ff::point_float(pos.x, pos.y);
                transform.scale = ff::point_float(p.size, p.size);
                transform.rotation = p.spin;
                p.anim->draw_frame(draw, transform, p.timer * p.anim->frames_per_second());
            }
        }
    }
}

bool retron::particles::effect_active(int effect_id) const
{
    for (auto& group : this->groups)
    {
        if (group.effect_id == effect_id && group.refs != -1)
        {
            return true;
        }
    }

    return false;
}

void retron::particles::effect_position(int effect_id, ff::point_fixed pos)
{
    for (auto& group : this->groups)
    {
        if (group.effect_id == effect_id && group.refs != -1)
        {
            group.transform.position = pos;
            DirectX::XMStoreFloat4x4(&group.matrix, group.transform.matrix());
        }
    }
}

template<typename ValueT, typename T = typename ff::type::value_derived_traits<ValueT>::raw_type>
static std::pair<T, T> read_pair(const ff::dict& dict, std::string_view name, T default1, T default2, bool* has_value)
{
    bool stack_has_value;
    has_value = has_value ? has_value : &stack_has_value;
    *has_value = false;

    ff::value_ptr value = dict.get(name);
    if (value)
    {
        ff::value_ptr valuet = value->try_convert<ValueT>();
        if (valuet)
        {
            *has_value = true;
            return std::make_pair(valuet->get<ValueT>(), valuet->get<ValueT>());
        }

        ff::value_ptr vector = value->try_convert<std::vector<ff::value_ptr>>();
        if (vector && vector->get<std::vector<ff::value_ptr>>().size() == 1)
        {
            valuet = vector->get<std::vector<ff::value_ptr>>()[0]->try_convert<ValueT>();
            if (valuet)
            {
                *has_value = true;
                return std::make_pair(valuet->get<ValueT>(), valuet->get<ValueT>());
            }
        }

        if (vector && vector->get<std::vector<ff::value_ptr>>().size() > 1)
        {
            ff::value_ptr v0 = vector->get<std::vector<ff::value_ptr>>()[0]->try_convert<ValueT>();
            ff::value_ptr v1 = vector->get<std::vector<ff::value_ptr>>()[1]->try_convert<ValueT>();
            if (v0 && v1)
            {
                *has_value = true;
                return std::make_pair(v0->get<ValueT>(), v1->get<ValueT>());
            }
        }
    }

    return std::make_pair(default1, default2);
}

retron::particles::spec_t::spec_t(const ff::dict& dict)
{
    this->count = ::read_pair<int>(dict, "count", 32, 32, nullptr);
    this->delay = ::read_pair<int>(dict, "delay", 0, 0, nullptr);
    this->life = ::read_pair<int>(dict, "life", 32, 32, nullptr);

    this->dist = ::read_pair<ff::fixed_int>(dict, "distance", 0, 0, nullptr);
    this->dist_vel = ::read_pair<ff::fixed_int>(dict, "velocity", 1, 1, nullptr);
    this->size = ::read_pair<ff::fixed_int>(dict, "size", 1, 1, nullptr);
    this->angle = ::read_pair<ff::fixed_int>(dict, "angle", 0, 360, &has_angle);
    this->angle_vel = ::read_pair<ff::fixed_int>(dict, "angle_velocity", 0, 0, nullptr);
    this->spin = ::read_pair<ff::fixed_int>(dict, "spin", 0, 0, nullptr);
    this->spin_vel = ::read_pair<ff::fixed_int>(dict, "spin_velocity", 0, 0, nullptr);

    this->rotate = dict.get<ff::fixed_int>("rotate");
    this->scale = dict.get<ff::point_fixed>("scale", ff::point_fixed(1, 1));
    this->reverse = dict.get<bool>("reverse");
    this->colors = dict.get<std::vector<int>>("colors");

    for (ff::value_ptr value : dict.get<std::vector<ff::value_ptr>>("animations"))
    {
        ff::value_ptr valuet = value->try_convert<ff::resource>();
        if (valuet)
        {
            ff::auto_resource<ff::animation_base> res = valuet->get<ff::resource>();
            std::shared_ptr<ff::animation_base> anim = res.object();
            assert(anim);

            if (anim)
            {
                this->animations.push_back(anim);
            }
        }
    }
}

void retron::particles::spec_t::add(particles& particles, ff::point_fixed pos, int effect_id, const retron::particles::effect_options& options) const
{
    const int count = ff::math::random_range(this->count);
    if (count <= 0)
    {
        return;
    }

    unsigned short group_id = particles.add_group(ff::pixel_transform(pos, scale * options.scale, rotate + options.rotate), effect_id, count, this->animations);

    for (int i = 0; i < count; i++)
    {
        retron::particles::particle_t p;

        p.angle = ff::math::degrees_to_radians((float)ff::math::random_range(has_angle ? angle : options.angle));
        p.angle_vel = ff::math::degrees_to_radians((float)ff::math::random_range(angle_vel));
        p.dist = ff::math::random_range(dist);
        p.dist_vel = ff::math::random_range(dist_vel);

        p.size = ff::math::random_range(size);
        p.spin = ff::math::random_range(spin);
        p.spin_vel = ff::math::random_range(spin_vel);
        p.timer = 0;

        p.delay = ff::math::random_range(delay);
        p.life = ff::math::random_range(life);
        p.type = options.type;
        p.internal_type = 0;
        p.group = group_id;

        if (this->animations.size())
        {
            const std::shared_ptr<ff::animation_base>& anim = this->animations[ff::math::random_range(0u, this->animations.size() - 1)];
            p.animation(anim);
        }
        else
        {
            p.size /= 2.0f;

            int color = this->colors.size() ? this->colors[ff::math::random_range(0u, this->colors.size() - 1)] : 0;
            p.color(color);
        }

        if ((reverse ^ options.reverse) != 0)
        {
            p.delay = (delay.second + life.second) - (p.delay + p.life);
            p.dist += p.dist_vel * p.life;
            p.dist_vel = -p.dist_vel;
            p.angle += p.angle_vel * p.life;
            p.angle_vel = -p.angle_vel;
        }

        particles.particles_new.push_back(p);
    }
}

retron::particles::effect_t::effect_t(const ff::value* value)
{
    std::vector<ff::value_ptr> dict_values;

    ff::value_ptr dict_value = value->try_convert<ff::dict>();
    ff::value_ptr vector_value = value->try_convert<std::vector<ff::value_ptr>>();

    if (dict_value)
    {
        dict_values.push_back(dict_value);
    }
    else if (vector_value)
    {
        for (ff::value_ptr value2 : vector_value->get<std::vector<ff::value_ptr>>())
        {
            dict_value = value2->try_convert<ff::dict>();
            if (dict_value)
            {
                dict_values.push_back(dict_value);
            }
        }
    }
    else
    {
        assert(false);
    }

    for (ff::value_ptr value2 : dict_values)
    {
        this->specs.emplace_back(value2->get<ff::dict>());
    }
}

int retron::particles::effect_t::add(particles& particles, ff::point_fixed pos, const retron::particles::effect_options* options) const
{
    return this->add(particles, &pos, 1, options);
}

int retron::particles::effect_t::add(particles& particles, const ff::point_fixed* pos, size_t posCount, const retron::particles::effect_options* options) const
{
    static std::atomic_int s_effect_id;
    int effect_id = s_effect_id.fetch_add(1) + 1;

    static retron::particles::effect_options default_options;
    options = options ? options : &default_options;

    for (const spec_t& spec : this->specs)
    {
        spec.add(particles, *pos, effect_id, *options);

        if (posCount > 1)
        {
            posCount--;
            pos++;
        }
    }

    return effect_id;
}
