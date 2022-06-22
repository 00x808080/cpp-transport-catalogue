#include "json_builder.h"
#include <iostream>
#include <utility>

using namespace std;
using namespace json;

// --------------- Builder ------------------

KeyItemContext Builder::Key(const std::string key) {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && targetKeyWasUsed_) {
        targetKey_ = key;
        targetKeyWasUsed_ = false;
    }
    else {
        throw logic_error("Key has not created"s);
    }
    return {*this};
}

Builder& Builder::Value(Node::Value value) {
    if (nodes_stack_.empty() && rootCreationCompleted) {
        throw logic_error("Value has not created"s);
    }

    if (nodes_stack_.empty()) {
        root_.GetValue() = value;
        nodes_stack_.push_back(&root_);
        rootCreationCompleted = true;
    }
    else if (nodes_stack_.back()->IsArray()) {
        Node* targetValue = new Node{};
        targetValue->GetValue() = move(value);
        nodes_stack_.back()->AsArray().emplace_back(move(*targetValue));
        delete targetValue;
    }
    else if (nodes_stack_.back()->IsDict() && !targetKeyWasUsed_) {
        nodes_stack_.back()->AsDict()[targetKey_].GetValue() = move(value);
        targetKeyWasUsed_ = true;
    }
    else {
        throw logic_error("Value has not created"s);
    }

    return *this;
}

DictItemContext Builder::StartDict() {
    if (nodes_stack_.empty() && rootCreationCompleted) {
        throw logic_error("Dict has not created"s);
    }

    if (nodes_stack_.empty()) {
        root_.GetValue() = Dict{};
        nodes_stack_.push_back(&root_);
    }
    else if (nodes_stack_.back()->IsArray()) {
        Node targetValue{Dict{}};
        nodes_stack_.back()->AsArray().push_back(targetValue);
        nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());
    }
    else if (nodes_stack_.back()->IsDict() && !targetKeyWasUsed_) {
        Node targetValue{Dict{}};
        nodes_stack_.back()->AsDict().insert({targetKey_, targetValue});
        nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(targetKey_));
        targetKeyWasUsed_ = true;
    }
    else {
        throw logic_error("Dict has not created"s);
    }

    return DictItemContext{*this};
}

ArrayItemContext Builder::StartArray() {
    if (nodes_stack_.empty() && rootCreationCompleted) {
        throw logic_error("Array has not created"s);
    }

    if (nodes_stack_.empty()) {
        root_.GetValue() = Array{};
        nodes_stack_.push_back(&root_);
        rootCreationCompleted = true;
    }
    else if (nodes_stack_.back()->IsArray()) {
        Node targetValue{Array {}};
        nodes_stack_.back()->AsArray().push_back(targetValue);
        nodes_stack_.push_back(&nodes_stack_.back()->AsArray().back());

    }
    else if (nodes_stack_.back()->IsDict() && !targetKeyWasUsed_) {
        Node targetValue{Array {}};
        nodes_stack_.back()->AsDict().insert({targetKey_, targetValue});
        nodes_stack_.push_back(&nodes_stack_.back()->AsDict().at(targetKey_));
        targetKeyWasUsed_ = true;
    }
    else {
        throw logic_error("Array has not created"s);
    }
    return ArrayItemContext{*this};
}

Builder &Builder::EndDict() {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict() && targetKeyWasUsed_) {
        if (nodes_stack_.size() == 1u) {
            rootCreationCompleted = true;
            return {*this};
        }
        nodes_stack_.pop_back();
    }
    else {
        throw logic_error("Dict closing error"s);
    }
    return {*this};
}

Builder &Builder::EndArray() {
    if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
        if (nodes_stack_.size() == 1u) {
            rootCreationCompleted = true;
            return {*this};
        }
        nodes_stack_.pop_back();
    }
    else {
        throw logic_error("Array closing error"s);
    }
    return {*this};
}

json::Node Builder::Build() {
    if (nodes_stack_.size() != 1 || !rootCreationCompleted) {
        throw logic_error("JSON has not built"s);
    }

    return root_;
}

// -------------- KeyItemContext -----------------

ArrayItemContext KeyItemContext::StartArray() {
    return builder_.StartArray();
}

DictItemContext KeyItemContext::StartDict() {
    return builder_.StartDict();
}

KeyValueItemContext KeyItemContext::Value(Node::Value value) {
    return builder_.Value(value);
}

// -------------- KeyValueItemContext -----------------

KeyItemContext KeyValueItemContext::Key(const string &key) {
    return builder_.Key(key);
}

Builder &KeyValueItemContext::EndDict() {
    return builder_.EndDict();
}

// -------------- ValueInArrayItemContext -----------------

ValueInArrayItemContext ValueInArrayItemContext::Value(Node::Value value) {
    return builder_.Value(value);
}

DictItemContext ValueInArrayItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext ValueInArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder ValueInArrayItemContext::EndArray() {
    return builder_.EndArray();
}

// -------------- DictItemContext -----------------

KeyItemContext DictItemContext::Key(const string &key) {
    return builder_.Key(key);
}

Builder &DictItemContext::EndDict() {
    return builder_.EndDict();
}

// -------------- ArrayItemContext -----------------

ValueInArrayItemContext ArrayItemContext::Value(Node::Value value) {
    return builder_.Value(value);
}

DictItemContext ArrayItemContext::StartDict() {
    return builder_.StartDict();
}

ArrayItemContext ArrayItemContext::StartArray() {
    return builder_.StartArray();
}

Builder &ArrayItemContext::EndArray() {
    return builder_.EndArray();
}
