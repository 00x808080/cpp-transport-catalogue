#pragma once

#include "json.h"

namespace json {

class KeyItemContext;
class KeyValueItemContext;
class ValueInArrayItemContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    KeyItemContext Key(std::string key);
    Builder& Value(Node::Value value);
    DictItemContext StartDict();
    Builder& EndDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

    json::Node Build();

private:
    bool rootCreationCompleted = false;
    bool targetKeyWasUsed_ = true;
    std::string targetKey_;
    Node root_;
    std::vector<Node*> nodes_stack_;
};

class KeyItemContext {
public:
    KeyItemContext(Builder& builder) : builder_(builder) {}

    ArrayItemContext StartArray();
    DictItemContext StartDict();
    KeyValueItemContext Value(Node::Value value);

private:
    Builder& builder_;
};

class KeyValueItemContext {
public:
    KeyValueItemContext(Builder& builder) : builder_(builder) {}

    KeyItemContext Key(const std::string& key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class ValueInArrayItemContext {
public:
    ValueInArrayItemContext(Builder& builder) : builder_(builder) {}

    ValueInArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder EndArray();

private:
    Builder& builder_;
};

class DictItemContext {
public:
    DictItemContext(Builder& builder) : builder_(builder) {}

    KeyItemContext Key(const std::string &key);
    Builder& EndDict();

private:
    Builder& builder_;
};

class ArrayItemContext {
public:
    ArrayItemContext(Builder& builder) : builder_(builder) {}

    ValueInArrayItemContext Value(Node::Value value);
    DictItemContext StartDict();
    ArrayItemContext StartArray();
    Builder& EndArray();

private:
    Builder& builder_;
};

} // namespace json