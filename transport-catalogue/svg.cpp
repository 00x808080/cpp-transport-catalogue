#include "svg.h"

namespace svg {

using namespace std::literals;

std::ostream& operator<<(std::ostream& out, const StrokeLineCap& line_cap) {
    if (line_cap == StrokeLineCap::BUTT) {
        out << "butt";
    }
    if (line_cap == StrokeLineCap::ROUND) {
        out << "round";
    }
    if (line_cap == StrokeLineCap::SQUARE) {
        out << "square";
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const StrokeLineJoin& line_join) {
    if (line_join == StrokeLineJoin::ARCS) {
        out << "arcs";
    }
    if (line_join == StrokeLineJoin::BEVEL) {
        out << "bevel";
    }
    if (line_join == StrokeLineJoin::MITER) {
        out << "miter";
    }
    if (line_join == StrokeLineJoin::MITER_CLIP) {
        out << "miter-clip";
    }
    if (line_join == StrokeLineJoin::ROUND) {
        out << "round";
    }
    return out;
}

// ---------- Object ------------------

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

// ---------- Circle ------------------

Circle& Circle::SetCenter(Point center)  {
    center_ = center;
    return *this;
}

Circle& Circle::SetRadius(double radius)  {
    radius_ = radius;
    return *this;
}

void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Polyline ----------------

Polyline& Polyline::AddPoint(Point point) {
    vertices_.push_back(point);
    return *this;
}

void Polyline::RenderObject(const RenderContext &context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;
    for (size_t i = 0; i < vertices_.size(); i++) {
        if (i == vertices_.size() - 1) {
            out << vertices_[i].x << ',' << vertices_[i].y;
            break;
        }
        out << vertices_[i].x << ',' << vertices_[i].y << ' ';
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ---------- Text -------------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    size_ = size;
    return *this;
}

Text &Text::SetFontFamily(std::string font_family) {
    font_family_ = font_family;
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = font_weight;
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = data;
    return *this;
}

void Text::RenderObject(const RenderContext &context) const {
    auto& out = context.out;
    out << "<text";
    RenderAttrs(out);
    out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv
        << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv
        << "font-size=\""sv << size_ << "\""sv;
    if (!font_family_.empty()) {
        out << " font-family=\""sv << font_family_ << "\""sv;
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\""sv << font_weight_ << "\""sv;
    }
    out << ">"sv << data_ << "</text>"sv;
}

// ---------- Document ---------------

void Document::AddPtr(std::unique_ptr<Object> &&obj) {
    objects_.emplace_back(std::move(obj));
}

void Document::Render(std::ostream &out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";
    for (auto& object : objects_) {
        object->Render(out);
    }
    out << "</svg>";
}

}  // namespace svg