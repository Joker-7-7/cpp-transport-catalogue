#pragma once

/**
 * @file json.h
 * Lightweight JSON parser and printer.
 *
 * Provides a DOM-style JSON representation: a @ref Node is a variant
 * over all JSON value types, and a @ref Document wraps the root node.
 * Parsing is done via json::Load(), printing via json::Print().
 */

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

/**
 * JSON parsing and printing utilities.
 */
namespace json {

class Node;
/**
 * A JSON object: ordered map of string → Node.
 */using Dict = std::map<std::string, Node>;
/**
 * A JSON array: ordered list of Nodes.
 */using Array = std::vector<Node>;

/**
 * Exception thrown when JSON parsing fails.
 */
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

/**
 * Represents a single JSON value.
 *
 * A Node wraps a std::variant of all possible JSON types:
 * null, Array, Dict, bool, int, double, and std::string.
 * Provides type-checking and value-access methods.
 */
class Node final {
public:
    /**
     * The variant type holding any JSON value.
     */    using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

    /**
     * @name Constructors
     */
    ///@{
    /**
     * Constructs a null Node.
     */    Node() = default;
    /**
     * Constructs a null Node.
     */    Node(std::nullptr_t)
        : value_(nullptr) {
    }
    /**
     * Constructs an integer Node.
     */    Node(int val)
        : value_(val) {
    }
    /**
     * Constructs a double Node.
     */    Node(double val)
        : value_(val) {
    }
    /**
     * Constructs a string Node.
     */    Node(std::string val)
        : value_(std::move(val)) {
    }
    /**
     * Constructs an array Node.
     */    Node(Array val)
        : value_(std::move(val)) {
    }
    /**
     * Constructs an object (map) Node.
     */    Node(Dict val)
        : value_(std::move(val)) {
    }
    /**
     * Constructs a boolean Node.
     */    Node(bool val)
        : value_(val) {
    }
    ///@}

    /**
     * @name Type checks
     */
    ///@{
    /**
     * Returns true if this node holds an int.
     */    bool IsInt() const {
        return std::holds_alternative<int>(value_);
    }
    /**
     * Returns true if this node holds a double (but not an int).
     */    bool IsPureDouble() const {
        return std::holds_alternative<double>(value_);
    }
    /**
     * Returns true if this node holds an int or a double.
     */    bool IsDouble() const {
        return IsInt() || IsPureDouble();
    }
    /**
     * Returns true if this node holds a bool.
     */    bool IsBool() const {
        return std::holds_alternative<bool>(value_);
    }
    /**
     * Returns true if this node holds null.
     */    bool IsNull() const {
        return std::holds_alternative<std::nullptr_t>(value_);
    }
    /**
     * Returns true if this node holds an array.
     */    bool IsArray() const {
        return std::holds_alternative<Array>(value_);
    }
    /**
     * Returns true if this node holds a string.
     */    bool IsString() const {
        return std::holds_alternative<std::string>(value_);
    }
    /**
     * Returns true if this node holds a map (object).
     */    bool IsMap() const {
        return std::holds_alternative<Dict>(value_);
    }
    ///@}

    /** @name Value accessors
     * Each throws std::logic_error if the Node does not hold the requested type.
     */
    ///@{
    /**
     * Returns the int value. Throws if not an int.
     */    int AsInt() const {
        using namespace std::literals;
        if (!IsInt()) {
            throw std::logic_error("Not an int"s);
        }
        return std::get<int>(value_);
    }
    /**
     * Returns the value as double (int is implicitly promoted). Throws if not numeric.
     */    double AsDouble() const {
        using namespace std::literals;
        if (!IsDouble()) {
            throw std::logic_error("Not a double"s);
        }
        return IsPureDouble() ? std::get<double>(value_) : AsInt();
    }
    /**
     * Returns the bool value. Throws if not a bool.
     */    bool AsBool() const {
        using namespace std::literals;
        if (!IsBool()) {
            throw std::logic_error("Not a bool"s);
        }
        return std::get<bool>(value_);
    }
    /**
     * Returns a const reference to the array. Throws if not an array.
     */    const Array& AsArray() const {
        using namespace std::literals;
        if (!IsArray()) {
            throw std::logic_error("Not an array"s);
        }
        return std::get<Array>(value_);
    }
    /**
     * Returns a const reference to the string. Throws if not a string.
     */    const std::string& AsString() const {
        using namespace std::literals;
        if (!IsString()) {
            throw std::logic_error("Not a string"s);
        }
        return std::get<std::string>(value_);
    }
    /**
     * Returns a const reference to the map. Throws if not a map.
     */    const Dict& AsMap() const {
        using namespace std::literals;
        if (!IsMap()) {
            throw std::logic_error("Not a map"s);
        }
        return std::get<Dict>(value_);
    }
    ///@}

    /**
     * Equality comparison of two Nodes.
     */
    bool operator==(const Node& rhs) const {
        return value_ == rhs.value_;
    }

    /**
     * Returns a const reference to the underlying variant.
     */
    const Value& GetValue() const {
        return value_;
    }

    /**
     * Returns a mutable reference to the underlying variant.
     */
    Value& GetValue() {
        return value_;
    }

private:
    Value value_;
};

/**
 * Inequality comparison of two Nodes.
 */
inline bool operator!=(const Node& lhs, const Node& rhs) {
    return !(lhs == rhs);
}

/**
 * A JSON document wrapping the root Node.
 */
class Document {
public:
    /**
     * Constructs a document from a root node.
     * @param root The root JSON node.
     */
    explicit Document(Node root)
        : root_(std::move(root)) {
    }

    /**
     * Returns a const reference to the root node.
     */
    const Node& GetRoot() const {
        return root_;
    }

private:
    Node root_;
};

/**
 * Equality comparison of two Documents (compares their root nodes).
 */
inline bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

/**
 * Inequality comparison of two Documents.
 */
inline bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

/**
 * Parses a JSON document from an input stream.
 * @param input The input stream to read from.
 * @return The parsed Document.
 * @throws ParsingError on malformed JSON.
 */
Document Load(std::istream& input);

/**
 * Serializes a JSON document to an output stream.
 * @param doc The document to print.
 * @param output The output stream to write to.
 */
void Print(const Document& doc, std::ostream& output);

}  // namespace json