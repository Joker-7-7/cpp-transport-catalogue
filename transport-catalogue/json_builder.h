#pragma once

/**
 * @file json_builder.h
 * Fluent interface for constructing JSON documents.
 *
 * The Builder class provides a domain-specific language for building JSON
 * objects and arrays with compile-time context validation. Special context
 * classes (KeyItemContext, DictItemContext, ArrayItemContext) restrict which
 * methods are callable at each stage, preventing malformed JSON.
 */

#include "json.h"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace json {

/**
 * Fluent JSON builder with context-sensitive method chaining.
 *
 * Usage example:
 * @code
 *   Builder{}.StartDict()
 *              .Key("name").Value("John")
 *              .Key("scores").StartArray()
 *                                .Value(100)
 *                                .Value(200)
 *                             .EndArray()
 *            .EndDict()
 *            .Build();
 * @endcode
 *
 * The context classes returned by StartDict(), StartArray(), and Key()
 * enforce that only valid operations are available at each stage.
 */
class Builder {
public:
    // Forward declarations of context classes.
    class KeyItemContext;
    class DictItemContext;
    class ArrayItemContext;

public:
    /**
     * Constructs an empty builder.
     */    Builder() = default;

    /**
     * Inserts a key into the current dictionary.
     * @param key The key name.
     * @return A KeyItemContext that allows setting the corresponding value.
     */
    KeyItemContext Key(std::string key);

    /**
     * Inserts a value into the current array or as a key's value.
     * @param value The JSON value to insert.
     * @return Reference to this builder for further chaining.
     */
    Builder& Value(Node value);

    /**
     * Starts a new nested JSON object.
     * @return A DictItemContext for adding key-value pairs into the new object.
     */
    DictItemContext StartDict();

    /**
     * Ends the current JSON object.
     * @return Reference to this builder.
     */
    Builder& EndDict();

    /**
     * Starts a new nested JSON array.
     * @return An ArrayItemContext for adding items into the new array.
     */
    ArrayItemContext StartArray();

    /**
     * Ends the current JSON array.
     * @return Reference to this builder.
     */
    Builder& EndArray();

    /**
     * Finalizes construction and returns the built JSON Node.
     * @return The root Node of the constructed JSON structure.
     * @throws std::logic_error if the builder is in an incomplete state.
     */
    Node Build();

private:
    /**
     * Adds a node to the current container (array or dict value).
     */    Node* AddNode(Node node);

    /**
     * Checks whether the top-level JSON value is complete.
     */    bool IsComplete() const;

    // Implementation methods called by context classes.
    void KeyImpl(std::string key);
    Builder& ValueImpl(Node value);
    Node* StartDictImpl();
    Builder& EndDictImpl();
    Node* StartArrayImpl();
    Builder& EndArrayImpl();

private:
    std::unique_ptr<Node> root_;
    std::vector<Node*> nodes_stack_;
    std::optional<std::string> key_;
};

/**
 * Context returned by Builder::Key().
 *
 * Allows setting a value, starting a dict, or starting an array
 * for the previously specified key.
 */
class Builder::KeyItemContext {
public:
    /**
     * Constructs a KeyItemContext.
     * @param builder Reference to the owning Builder.
     */
    explicit KeyItemContext(Builder& builder)
        : builder_(builder) {
    }

    /**
     * Sets a primitive value for the key.
     * @return A DictItemContext for further key-value pairs in the parent dict.
     */
    DictItemContext Value(Node value);

    /**
     * Starts a nested dict as the value for the key.
     * @return A DictItemContext for adding key-value pairs.
     */
    DictItemContext StartDict();

    /**
     * Starts a nested array as the value for the key.
     * @return An ArrayItemContext for adding items.
     */
    ArrayItemContext StartArray();

private:
    Builder& builder_;
};

/**
 * Context returned by Builder::StartDict() or KeyItemContext::Value().
 *
 * Allows adding more key-value pairs or ending the current dictionary.
 */
class Builder::DictItemContext {
public:
    /**
     * Constructs a DictItemContext.
     * @param builder Reference to the owning Builder.
     */
    explicit DictItemContext(Builder& builder)
        : builder_(builder) {
    }

    /**
     * Adds a new key to the current dictionary.
     * @return A KeyItemContext for setting the key's value.
     */
    KeyItemContext Key(std::string key);

    /**
     * Ends the current dictionary.
     * @return Reference to the Builder.
     */
    Builder& EndDict();

private:
    Builder& builder_;
};

/**
 * Context returned by Builder::StartArray() or ArrayItemContext::Value().
 *
 * Allows adding values, starting nested dicts/arrays, or ending the current array.
 */
class Builder::ArrayItemContext {
public:
    /**
     * Constructs an ArrayItemContext.
     * @param builder Reference to the owning Builder.
     */
    explicit ArrayItemContext(Builder& builder)
        : builder_(builder) {
    }

    /**
     * Adds a value to the current array.
     * @return Another ArrayItemContext for further items.
     */
    ArrayItemContext Value(Node value);

    /**
     * Starts a nested dict as an array element.
     * @return A DictItemContext for adding key-value pairs.
     */
    DictItemContext StartDict();

    /**
     * Starts a nested array as an array element.
     * @return An ArrayItemContext for adding items.
     */
    ArrayItemContext StartArray();

    /**
     * Ends the current array.
     * @return Reference to the Builder.
     */
    Builder& EndArray();

private:
    Builder& builder_;
};

}