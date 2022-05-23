// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "../GUI/gui_system.hpp"
#include "../GFX/gfx_surface_delegate_vulkan.hpp"
#include "widget.hpp"
#include <vulkan/vulkan.hpp>

namespace hi::inline v1 {

/** A widget that draws directly into the swap-chain.
 */
class vulkan_widget : public widget, public gfx_surface_delegate_vulkan {
public:
    using super = widget;

    vulkan_widget(hi::gui_window& window, hi::widget *parent) noexcept : super(window, parent)
    {
        window.surface->add_delegate(this);
    }

    ~vulkan_widget()
    {
        window.surface->remove_delegate(this);
    }

    widget_constraints const& set_constraints() noexcept override
    {
        _layout = {};
        return _constraints = {{100, 50}, {200, 100}, {300, 100}, theme().margin};
    }

    void set_layout(hi::widget_layout const& layout) noexcept override
    {
        if (compare_store(_layout, layout)) {}
    }

    

    void draw(hi::draw_context const& context) noexcept override
    {
        if (*visible and overlaps(context, layout())) {
            context.make_hole(_layout, _layout.rectangle());

            draw_vulkan(aarectangle{_layout.to_window * _layout.rectangle()}, context.scissor_rectangle);
        }
    }
};

} // namespace hi::inline v1
