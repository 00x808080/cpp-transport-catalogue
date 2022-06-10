#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

class Node;
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node {
public:
    Node() = default;

    Node(nullptr_t);
    Node(bool value);
    Node(Array array);
    Node(Dict map);
    Node(int value);
    Node(double value);
    Node(std::string value);

    bool IsNull() const;
    bool IsBool() const;
    bool AsBool() const;
    bool IsArray() const;
    const Array& AsArray() const;
    bool IsMap() const;
    const Dict& AsMap() const;
    bool IsInt() const;
    int AsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    double AsDouble() const;
    bool IsString() const;
    const std::string& AsString() const;

    const NodeValue& GetValue() const;

private:
    NodeValue value_ = {};
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

    bool operator==(const Document& rhs) const;
    bool operator!=(const Document& rhs) const;

private:
    Node root_;
};

Document Load(std::istream& input);

bool operator==(const Node& lhs, const Node& rhs);
bool operator!=(const Node& lhs, const Node& rhs);
bool operator==(const Array& lhs, const Array& rhs);
bool operator!=(const Array& lhs, const Array& rhs);
bool operator==(const Dict& lhs, const Dict& rhs);
bool operator!=(const Dict& lhs, const Dict& rhs);

void PrintNode(std::ostream& out, nullptr_t);
void PrintNode(std::ostream& out, const Array& array);
void PrintNode(std::ostream& out, const Dict& dict);
void PrintNode(std::ostream& out, const int& value);
void PrintNode(std::ostream& out, const double& value);
void PrintNode(std::ostream& out, const std::string& str);
void PrintNode(std::ostream& out, const bool& value);

void Print(const Document& doc, std::ostream& output);

}  // namespace json