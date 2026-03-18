#pragma once

#include "json.h"

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace json {

class Builder {
public:
    class KeyItemContext;
    class DictItemContext;
    class ArrayItemContext;

public:
    Builder() = default;

    KeyItemContext Key(std::string key);
    Builder& Value(Node value);

    DictItemContext StartDict();
    Builder& EndDict();

    ArrayItemContext StartArray();
    Builder& EndArray();

    Node Build();

private:
    Node* AddNode(Node node);
    bool IsComplete() const;

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

class Builder::KeyItemContext {
public:
    explicit KeyItemContext(Builder& builder)
        : builder_(builder) {
    }

    DictItemContext Value(Node value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();

private:
    Builder& builder_;
};

class Builder::DictItemContext {
public:
    explicit DictItemContext(Builder& builder)
        : builder_(builder) {
    }

    KeyItemContext Key(std::string key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class Builder::ArrayItemContext {
public:
    explicit ArrayItemContext(Builder& builder)
        : builder_(builder) {
    }

    ArrayItemContext Value(Node value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

private:
    Builder& builder_;
};

}  