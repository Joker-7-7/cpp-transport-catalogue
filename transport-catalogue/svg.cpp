#include "svg.h"

namespace svg {

using namespace std::literals;
namespace {
    static std::string EscapeText(const std::string& text)
    {
        std::string result;
        result.reserve(text.size());

        for (char c : text) {
            switch (c) {
                case '"':
                    result += "&quot;";
                    break;
                case '\'':
                    result += "&apos;";
                    break;
                case '<':
                    result += "&lt;";
                    break;
                case '>':
                    result += "&gt;";
                    break;
                case '&':
                    result += "&amp;";
                    break;
                default:
                    result += c;
                    break;
            }
        }

        return result;
    }
}

std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap) {
        switch (stroke_line_cap)
        {
        case StrokeLineCap::BUTT:
            out << "butt";
            break;
        case StrokeLineCap::ROUND:
            out << "round";
            break;
        case StrokeLineCap::SQUARE:
            out << "square";
            break;
        }
        return out;
}

std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join) 
{
    switch (stroke_line_join)
    {
    case StrokeLineJoin::ARCS:
        out << "arcs";
        break;
    case StrokeLineJoin::BEVEL:
        out << "bevel";
        break;
    case StrokeLineJoin::MITER:
        out << "miter";
        break;
    case StrokeLineJoin::MITER_CLIP:
        out << "miter-clip";
        break;
    case StrokeLineJoin::ROUND:
        out << "round";
        break;
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const Color& color) {
    std::visit(
        [&out](const Color& color) {
            
            if (holds_alternative<std::monostate>(color)) {
                out << "none";
            } else if(holds_alternative<std::string>(color)) {
                out << std::get<std::string>(color);
            } else if (holds_alternative<Rgb>(color)) {
                auto rgb = std::get<Rgb>(color);
                out << "rgb("sv
                    << static_cast<int>(rgb.red) << ","sv
                    << static_cast<int>(rgb.green) << ","sv
                    << static_cast<int>(rgb.blue) << ")"sv;
            } else if (holds_alternative<Rgba>(color)) {
                auto rgba = std::get<Rgba>(color);
                out << "rgba("sv
                    << static_cast<int>(rgba.red) << ","sv
                    << static_cast<int>(rgba.green) << ","sv
                    << static_cast<int>(rgba.blue) << ","sv
                    << rgba.opacity << ")"sv;
            }
        },
        color);

    return out;
}

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n";
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n";

    RenderContext ctx(out, 2, 2);
    for (const auto& object : objects_) {
        object->Render(ctx);
    }

    out << "</svg>";
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
    out << "r=\""sv << radius_ << "\" "sv;
    RenderAttrs(out);
    out << "/>"sv;
}

// ----------- Polyline -----------------
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<polyline points=\""sv;

    bool first = true;
    for (const auto& point : points_) {
        if (!first) {
            out << " ";
        }
        first = false;

        out << point.x << "," << point.y;
    }
    out << "\"";
    RenderAttrs(out);
    out << "/>"sv;
}

Polyline& Polyline::AddPoint(Point point) {
    points_.push_back(point);
    return *this;
}

// ----------- Text -----------------

Text& Text::SetPosition(Point pos) {
    pos_ = pos;
    return *this;
}

Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}

Text& Text::SetFontSize(uint32_t size) {
    font_size_ = size;
    return *this;
}

Text& Text::SetFontFamily(std::string font_family) {
    font_family_ = std::move(font_family);
    return *this;
}

Text& Text::SetFontWeight(std::string font_weight) {
    font_weight_ = std::move(font_weight);
    return *this;
}

Text& Text::SetData(std::string data) {
    data_ = std::move(data);
    return *this;
}

void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text";

    out << " x=\"" << pos_.x << "\"";
    out << " y=\"" << pos_.y << "\"";
    out << " dx=\"" << offset_.x << "\"";
    out << " dy=\"" << offset_.y << "\"";
    out << " font-size=\"" << font_size_ << "\"";
    RenderAttrs(out);
    if (!font_family_.empty()) {
        out << " font-family=\"" << font_family_ << "\"";
    }
    if (!font_weight_.empty()) {
        out << " font-weight=\"" << font_weight_ << "\"";
    }

    out << ">";
    out << EscapeText(data_);
    out << "</text>";
}

}  // namespace svg