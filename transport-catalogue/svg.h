#pragma once

/**
 * @file svg.h
 * Lightweight SVG document model and rendering.
 *
 * Provides an object-oriented API for constructing SVG documents:
 * primitives (Circle, Polyline, Text), shared path properties via CRTP
 * mixin (PathProps), an object container, and a Document that serializes
 * everything to an output stream.
 */

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <variant>

/**
 * SVG rendering primitives.
 */
namespace svg {

/**
 * RGB color representation (8 bits per channel).
 */
struct Rgb {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

/**
 * RGBA color representation with opacity.
 */
struct Rgba {
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
    /**
     * Opacity in range [0.0, 1.0].
     */    double opacity = 1.0;
};

/**
 * A color that can be "none" (monostate), a named string, RGB, or RGBA.
 */
using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

/**
 * Sentinel value representing "no color".
 */
inline const Color NoneColor{std::monostate{}};

/**
 * SVG stroke-linecap attribute values.
 */
enum class StrokeLineCap {
    BUTT,   ///< No projection beyond the line end.
    ROUND,  ///< Rounded cap.
    SQUARE, ///< Squared cap with projection.
};

/**
 * SVG stroke-linejoin attribute values.
 */
enum class StrokeLineJoin {
    ARCS,       ///< Arcs join.
    BEVEL,      ///< Bevel join.
    MITER,      ///< Miter join.
    MITER_CLIP, ///< Miter join with clipping.
    ROUND,      ///< Round join.
};

/**
 * Writes a Color to an output stream in SVG attribute format.
 */std::ostream& operator<<(std::ostream& out, const Color& color);
/**
 * Writes a StrokeLineCap enum value as an SVG string.
 */std::ostream& operator<<(std::ostream& out, StrokeLineCap stroke_line_cap);
/**
 * Writes a StrokeLineJoin enum value as an SVG string.
 */std::ostream& operator<<(std::ostream& out, StrokeLineJoin stroke_line_join);

/**
 * 2D point used for SVG coordinates.
 */
struct Point {
    Point() = default;
    /**
     * Constructs a point at (x, y).
     */
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0; ///< X-coordinate.
    double y = 0; ///< Y-coordinate.
};

/**
 * Rendering context holding the output stream and current indentation level.
 */
struct RenderContext {
    /**
     * Constructs a context for the given output stream.
     */
    RenderContext(std::ostream& out)
        : out(out) {
    }

    /**
     * Constructs a context with specified indentation.
     * @param out         The output stream.
     * @param indent_step Number of spaces per indent level.
     * @param indent      Current indent level.
     */
    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    /**
     * Returns a new context with one additional level of indentation.
     */
    RenderContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }

    /**
     * Writes the current indentation (spaces) to the output stream.
     */
    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;    ///< The output stream.
    int indent_step = 0;  ///< Spaces per indent level.
    int indent = 0;       ///< Current indent level.
};

/**
 * CRTP mixin providing fill/stroke SVG presentation attributes.
 *
 * Used by Circle, Polyline, and Text to share SetFillColor, SetStrokeColor,
 * SetStrokeWidth, SetStrokeLineCap, and SetStrokeLineJoin methods.
 *
 * @tparam Owner The derived class type.
 */
template <class Owner>
class PathProps {
public:
    /**
     * Sets the fill color (default: none).
     */
    Owner& SetFillColor(Color fill_color) {
        fill_color_ = std::move(fill_color);
        return AsOwner();
    }

    /**
     * Sets the stroke color (default: none).
     */
    Owner& SetStrokeColor(Color stroke_color) {
        stroke_color_ = std::move(stroke_color);
        return AsOwner();
    }

    /**
     * Sets the stroke width.
     */
    Owner& SetStrokeWidth(double stroke_width) {
        stroke_width_ = std::move(stroke_width);
        return AsOwner();
    }

    /**
     * Sets the stroke-linecap style.
     */
    Owner& SetStrokeLineCap(StrokeLineCap stroke_line_cap) {
        stroke_line_cap_ = std::move(stroke_line_cap);
        return AsOwner();
    }

    /**
     * Sets the stroke-linejoin style.
     */
    Owner& SetStrokeLineJoin(StrokeLineJoin stroke_line_join) {
        stroke_line_join_ = std::move(stroke_line_join);
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    /**
     * Renders all set presentation attributes to the output stream.
     */
    void RenderAttrs(std::ostream& out) const {
        using namespace std::literals;

        if (fill_color_) {
            out << " fill=\""sv << *fill_color_ << "\""sv;
        }
        if (stroke_color_) {
            out << " stroke=\""sv << *stroke_color_ << "\""sv;
        }
        if (stroke_width_) {
            out << " stroke-width=\""sv << *stroke_width_ << "\""sv;
        }
        if (stroke_line_cap_) {
            out << " stroke-linecap=\""sv << *stroke_line_cap_ << "\""sv;
        }
        if (stroke_line_join_) {
            out << " stroke-linejoin=\""sv << *stroke_line_join_ << "\""sv;
        }
    }

private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> stroke_line_cap_;
    std::optional<StrokeLineJoin> stroke_line_join_;
};

/**
 * Abstract base class for all drawable SVG objects.
 *
 * Provides the Render method that handles indentation and delegates
 * the actual rendering to RenderObject().
 */
class Object {
public:
    /**
     * Renders the object with the given context.
     */
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    /**
     * Subclass-specific rendering implementation.
     */
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/**
 * SVG `<circle>` element.
 * @see https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    /**
     * Sets the center of the circle.
     */
    Circle& SetCenter(Point center);

    /**
     * Sets the radius of the circle.
     */
    Circle& SetRadius(double radius);

private:
    void RenderObject(const RenderContext& context) const override;

    Point center_;
    double radius_ = 1.0;
};

/**
 * SVG `<polyline>` element.
 * @see https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline : public Object, public PathProps<Polyline> {
public:
    /**
     * Adds a vertex to the polyline.
     */
    Polyline& AddPoint(Point point);

private:
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

/**
 * SVG `<text>` element.
 * @see https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text : public Object, public PathProps<Text> {
public:
    /**
     * Sets the anchor position (x, y attributes).
     */
    Text& SetPosition(Point pos);

    /**
     * Sets the text offset (dx, dy attributes).
     */
    Text& SetOffset(Point offset);

    /**
     * Sets the font size (font-size attribute).
     */
    Text& SetFontSize(uint32_t size);

    /**
     * Sets the font family (font-family attribute).
     */
    Text& SetFontFamily(std::string font_family);

    /**
     * Sets the font weight (font-weight attribute).
     */
    Text& SetFontWeight(std::string font_weight);

    /**
     * Sets the text content displayed inside the element.
     */
    Text& SetData(std::string data);

private:
    void RenderObject(const RenderContext& context) const override;

    Point pos_{0, 0};
    Point offset_{0, 0};
    uint32_t font_size_ = 1;
    std::string font_family_;
    std::string font_weight_;
    std::string data_;
};

/**
 * Abstract container for owning SVG objects.
 *
 * Stores objects as unique_ptr for polymorphism. Derived classes
 * define how objects are added.
 */
class ObjectContainer {
public:
    ObjectContainer() = default;

    ObjectContainer(const ObjectContainer&) = delete;
    ObjectContainer& operator=(const ObjectContainer&) = delete;

    ObjectContainer(ObjectContainer&&) = default;
    ObjectContainer& operator=(ObjectContainer&&) = default;

    /**
     * Adds an object by unique_ptr.
     */
    virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    /**
     * Adds an object by value (constructs it in-place).
     * @tparam Obj Concrete Object subclass.
     */
    template <typename Obj>
    void Add(Obj obj) {
       AddPtr(std::make_unique<Obj>(std::move(obj)));
    }

protected:
    virtual ~ObjectContainer() = default;

    std::vector<std::unique_ptr<Object>> objects_;
};

/**
 * Interface for objects that can draw themselves into an ObjectContainer.
 */
class Drawable {
public:
    /**
     * Draws this object by adding SVG primitives to the container.
     */
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

/**
 * Top-level SVG document.
 *
 * Owns all SVG objects and can render them to an output stream as a
 * complete SVG document.
 */
class Document : public ObjectContainer {
public:
    Document() = default;

    /**
     * Adds an object to the document (by unique_ptr).
     */
    void AddPtr(std::unique_ptr<Object>&& obj) override {
        objects_.emplace_back(std::move(obj));
    }

    /**
     * Renders the complete SVG document to an output stream.
     * @param out The stream to write to.
     */
    void Render(std::ostream& out) const;
};

}