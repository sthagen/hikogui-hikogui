// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/GUI/Widget.hpp"
#include "TTauri/GUI/utils.hpp"

namespace TTauri::GUI::Widgets {

Widget::Widget() noexcept
{
}

Device *Widget::device() const noexcept
{
    ttauri_assert(window);
    auto device = window->device;
    ttauri_assert(device);
    return device;
}

void Widget::setParent(Widget *parent) noexcept
{
    this->window = parent->window;
    this->parent = parent;
}

bool Widget::modified() const noexcept
{
    auto m = box.modified() || _modified;
    _modified = false;
    return m;
}

void Widget::update(
    bool modified,
    vspan<PipelineFlat::Vertex> &flat_vertices,
    vspan<PipelineImage::Vertex> &image_vertices,
    vspan<PipelineSDF::Vertex> &sdf_vertices) noexcept
{
    for (auto &child : children) {
        child->update(child->modified(), flat_vertices, image_vertices, sdf_vertices);
    }
}

HitBox Widget::hitBoxTest(glm::vec2 position) const noexcept
{
    for (auto& widget : children) {
        if (widget->box.contains(position)) {
            return widget->hitBoxTest(position);
        }
    }
    return HitBox::NoWhereInteresting;
}

void Widget::handleMouseEvent(MouseEvent const event) noexcept
{
    assert(event.type != MouseEvent::Type::None);

    let targetWidget = currentMouseTarget;
    if (targetWidget) {
        if (targetWidget->box.contains(event.position)) {
            targetWidget->handleMouseEvent(event);
            if (event.type == MouseEvent::Type::Exited) {
                currentMouseTarget = nullptr;
            }

            // We completed sending the mouse event to the correct widget.
            return;

        } else {
            // We exited the previous target widget, send a exited event.
            targetWidget->handleMouseEvent(ExitedMouseEvent(event.position));
            currentMouseTarget = nullptr;
        }
    }

    if (event.type == MouseEvent::Type::Exited) {
        // We do not have any targets to send this event.
        return;
    }

    for (auto& widget : children) {
        if (widget->box.contains(event.position)) {
            currentMouseTarget = widget.get();
            return widget->handleMouseEvent(event);
        }
    }
    window->setCursor(Cursor::Default);
}

}
