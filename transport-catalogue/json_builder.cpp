#include "json_builder.h"

#include <utility>

using namespace std::literals;

namespace json {

Builder::KeyItemContext Builder::Key(std::string key) {
    KeyImpl(std::move(key));
    return KeyItemContext(*this);
}

Builder& Builder::Value(Node value) {
    return ValueImpl(std::move(value));
}

Builder::DictItemContext Builder::StartDict() {
    StartDictImpl();
    return DictItemContext(*this);
}

Builder& Builder::EndDict() {
    return EndDictImpl();
}

Builder::ArrayItemContext Builder::StartArray() {
    StartArrayImpl();
    return ArrayItemContext(*this);
}

Builder& Builder::EndArray() {
    return EndArrayImpl();
}

Node Builder::Build() {
    if (!IsComplete()) {
        throw std::logic_error("Builder: object is not complete"s);
    }
    return *root_;
}

void Builder::KeyImpl(std::string key) {
    if (IsComplete()) {
        throw std::logic_error("Builder: object has already been built"s);
    }

    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Builder: Key() called outside of a dictionary"s);
    }

    if (key_.has_value()) {
        throw std::logic_error("Builder: Key() called while previous key has no value"s);
    }

    key_ = std::move(key);
}

Builder& Builder::ValueImpl(Node value) {
    if (IsComplete()) {
        throw std::logic_error("Builder: object has already been built"s);
    }

    if (root_ == nullptr) {
        root_ = std::make_unique<Node>(std::move(value));
        return *this;
    }

    if (nodes_stack_.empty()) {
        throw std::logic_error("Builder: Value() called in invalid context"s);
    }

    Node* current = nodes_stack_.back();

    if (current->IsArray()) {
        std::get<Array>(current->GetValue()).emplace_back(std::move(value));
        return *this;
    }

    if (current->IsMap()) {
        if (!key_.has_value()) {
            throw std::logic_error("Builder: Value() called in dictionary without key"s);
        }

        auto& dict = std::get<Dict>(current->GetValue());
        dict.emplace(std::move(*key_), Node(std::move(value)));
        key_.reset();
        return *this;
    }

    throw std::logic_error("Builder: Value() called in invalid context"s);
}

Node* Builder::StartDictImpl() {
    if (IsComplete()) {
        throw std::logic_error("Builder: object has already been built"s);
    }

    Node* inserted = AddNode(Dict{});
    nodes_stack_.push_back(inserted);
    return inserted;
}

Builder& Builder::EndDictImpl() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsMap()) {
        throw std::logic_error("Builder: EndDict() called outside of a dictionary"s);
    }

    if (key_.has_value()) {
        throw std::logic_error("Builder: EndDict() called, but key has no value"s);
    }

    nodes_stack_.pop_back();
    return *this;
}

Node* Builder::StartArrayImpl() {
    if (IsComplete()) {
        throw std::logic_error("Builder: object has already been built"s);
    }

    Node* inserted = AddNode(Array{});
    nodes_stack_.push_back(inserted);
    return inserted;
}

Builder& Builder::EndArrayImpl() {
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw std::logic_error("Builder: EndArray() called outside of an array"s);
    }

    nodes_stack_.pop_back();
    return *this;
}

Node* Builder::AddNode(Node node) {
    if (root_ == nullptr) {
        root_ = std::make_unique<Node>(std::move(node));
        return root_.get();
    }

    if (nodes_stack_.empty()) {
        throw std::logic_error("Builder: cannot add a node in this context"s);
    }

    Node* current = nodes_stack_.back();

    if (current->IsArray()) {
        auto& array = std::get<Array>(current->GetValue());
        array.emplace_back(std::move(node));
        return &array.back();
    }

    if (current->IsMap()) {
        if (!key_.has_value()) {
            throw std::logic_error("Builder: cannot add a node to dictionary without key"s);
        }

        auto& dict = std::get<Dict>(current->GetValue());
        auto [it, inserted] = dict.emplace(std::move(*key_), std::move(node));
        key_.reset();
        return &it->second;
    }

    throw std::logic_error("Builder: invalid container state"s);
}

bool Builder::IsComplete() const {
    return root_ != nullptr && nodes_stack_.empty() && !key_.has_value();
}

Builder::DictItemContext Builder::KeyItemContext::Value(Node value) {
    builder_.ValueImpl(std::move(value));
    return DictItemContext(builder_);
}

Builder::DictItemContext Builder::KeyItemContext::StartDict() {
    builder_.StartDictImpl();
    return DictItemContext(builder_);
}

Builder::ArrayItemContext Builder::KeyItemContext::StartArray() {
    builder_.StartArrayImpl();
    return ArrayItemContext(builder_);
}

Builder::KeyItemContext Builder::DictItemContext::Key(std::string key) {
    builder_.KeyImpl(std::move(key));
    return KeyItemContext(builder_);
}

Builder& Builder::DictItemContext::EndDict() {
    return builder_.EndDictImpl();
}

Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node value) {
    builder_.ValueImpl(std::move(value));
    return ArrayItemContext(builder_);
}

Builder::DictItemContext Builder::ArrayItemContext::StartDict() {
    builder_.StartDictImpl();
    return DictItemContext(builder_);
}

Builder::ArrayItemContext Builder::ArrayItemContext::StartArray() {
    builder_.StartArrayImpl();
    return ArrayItemContext(builder_);
}

Builder& Builder::ArrayItemContext::EndArray() {
    return builder_.EndArrayImpl();
}

}  // namespace json