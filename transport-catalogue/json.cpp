#include "json.h"

using namespace std;

namespace json {

namespace {

// ------------- Load ----------------------

Node LoadNode(istream& input);

Node LoadArray(istream& input) {
    Array result;

    char c;
    while (input >> c && c != ']') {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    if (c != ']') throw ParsingError("Failed fill array"s);
    return Node(move(result));
}

Node LoadNumber(istream& input) {
    using namespace std::literals;

    std::string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!std::isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (std::isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    } else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        is_int = false;
        read_char();
        if (std::isdigit(input.peek())) read_digits();
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(stoi(parsed_num));
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
                return Node(stod(parsed_num));
            }
        }

    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
    return {parsed_num};
}

Node LoadString(istream& input) {
    string result;
    char c;
    while (input.get(c) && c != '"') {
        if (c == '\\') {
            if (input.peek() == 'r') result += '\r';
            else if (input.peek() == 'n') result += '\n';
            else if (input.peek() == 't') result += '\t';
            else if (input.peek() == '"') result += '\"';
            else if (input.peek() == '\\') result += '\\';
            else result += ' ';
            input.get();
            continue;
        }
        result += c;
    }

    if (c != '"') throw ParsingError("Незакрытая кавычка");
    return Node(move(result));
}

Node LoadDict(istream& input) {
    Dict result;
    char c;
    while (input >> c && c != '}') {
        if (c == ' ' || c == ',') continue;
        string key = LoadString(input).AsString();
        result.insert({move(key), LoadNode(input)});
    }

    if (c != '}') throw ParsingError("Failed fill dictionary"s);
    return Node(move(result));
}

Node LoadBool(istream& input) {
    string str;
    for (size_t i = 0; i < 4; i++) {
        str += input.get();
    }

    if (str == "true") return Node{true};
    str += input.get();
    if (str == "false") {
        return Node{false};
    }
    else {
        throw ParsingError("Failed read bool"s);
    }
}

Node LoadNull(istream& input) {
    string str;
    for (size_t i = 0; i < 4; i++) {
        str += input.get();
    }

    if (str == "null") return Node{};
    else {
        throw ParsingError("Failed read null"s);
    }
}

Node LoadNode(istream& input) {
    if (input.eof()) throw ParsingError("input is empty"s);

    char c;
    input.get(c);

    while (c == ' ' || c == ':' || c == '\t' || c == '\n' || c == '\r' ) input.get(c);

    if (c == 'n') {
        input.putback(c);
        return LoadNull(input);
    }
    else if (c == 't' || c == 'f') {
        input.putback(c);
        return LoadBool(input);
    }
    else if (c == '[') {
        return LoadArray(input);
    }
    else if (c == '{') {
        return LoadDict(input);
    }
    else if (c == '"') {
        return LoadString(input);
    }
    else {
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

// ------------- Node ----------------------

Node::Node(nullptr_t) {
    value_ = nullptr;
}

Node::Node(Array array)
        : value_(move(array)) {
}

Node::Node(Dict map)
        : value_(move(map)) {
}

Node::Node(int value)
        : value_(move(value)) {
}

Node::Node(double value)
        : value_(move(value)){
}

Node::Node(const string& value)
        : value_(move(value)) {
}

bool Node::IsNull() const {
    return holds_alternative<std::nullptr_t>(value_);
}

Node::Node(bool value)
        : value_(move(value)) {
}

bool Node::IsBool() const {
    return holds_alternative<bool>(value_);
}

bool Node::AsBool() const {
    try {
        return get<bool>(value_);
    }
    catch (...) {
        throw logic_error("Value is not bool");
    }
}

bool Node::IsArray() const {
    return holds_alternative<Array>(value_);
}

const Array& Node::AsArray() const {
    try {
        return get<Array>(value_);
    }
    catch (...) {
        throw logic_error("Value is not array");
    }
}

bool Node::IsMap() const {
    return holds_alternative<Dict>(value_);
}

const Dict& Node::AsMap() const {
    try {
        return get<Dict>(value_);
    }
    catch (...) {
        throw logic_error("Value is not dict");
    }
}

bool Node::IsInt() const {
    return holds_alternative<int>(value_);
}

int Node::AsInt() const {
    try {
        return get<int>(value_);
    }
    catch (...) {
        throw logic_error("Value is not int");
    }
}

bool Node::IsDouble() const {
    return holds_alternative<int>(value_) || holds_alternative<double>(value_);
}

bool Node::IsPureDouble() const {
    return holds_alternative<double>(value_);
}

double Node::AsDouble() const {
    try {
        if (holds_alternative<double>(value_)) {
            return get<double>(value_);
        }
        return get<int>(value_);
    }
    catch (...) {
        throw logic_error("Value is not double");
    }
}

bool Node::IsString() const {
    return holds_alternative<string>(value_);
}

const string& Node::AsString() const {
    try {
        return get<string>(value_);
    }
    catch (...) {
        throw logic_error("Value is not string");
    }
}

const NodeValue& Node::GetValue() const {
    return value_;
}



// ------------- Document ----------------------

Document::Document(const Node& root)
        : root_(move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool Document::operator==(const Document &rhs) const {
    return this->root_.GetValue() == rhs.root_.GetValue();
}

bool Document::operator!=(const Document &rhs) const {
    return this->root_.GetValue() != rhs.root_.GetValue();
}

// ---------------------------------------------

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

bool operator==(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() == rhs.GetValue();
}

bool operator!=(const Node& lhs, const Node& rhs) {
    return lhs.GetValue() != rhs.GetValue();
}

bool operator==(const Array& lhs, const Array& rhs) {
    if (lhs.size() != rhs.size()) return false;
    for (size_t i = 0; i < lhs.size(); i++) {
        if (lhs[i] != rhs[i]) return false;
    }
    return true;
}

bool operator!=(const Array& lhs, const Array& rhs) {
    return !(lhs == rhs);
}

bool operator==(const Dict& lhs, const Dict& rhs) {
    if (lhs.size() != rhs.size()) return false;
    auto it_lhs = begin(lhs);
    auto it_rhs = begin(rhs);
    while(it_lhs != lhs.end()) {
        if (it_lhs->first != it_rhs->first || it_lhs->second != it_rhs->second) {
            return false;
        }
        it_lhs++;
        it_rhs++;
    }
    return true;
}

bool operator!=(const Dict& lhs, const Dict& rhs) {
    return !(lhs == rhs);
}

// ------------- Print ----------------------

void PrintNode(std::ostream& out, nullptr_t) {
    out << "null";
}

void PrintNode(std::ostream& out, const Array& array) {
    out << "[\n";
    int counter = 0;
    int finish = array.size();
    for (const Node& node : array) {
        visit([&out](auto value) { PrintNode(out, value); }, node.GetValue());
        counter++;

        if (counter == finish) break;
        out << ", \n";
    }
    out << "\n]";
}

void PrintNode(std::ostream& out, const Dict& dict) {
    out << "{\n"s;
    int counter = 0;
    int finish = dict.size();
    for (const auto& key_to_node : dict) {
        out << "\""s << key_to_node.first << "\""s << ": "s;
        visit([&out](auto value) { PrintNode(out, value); }, key_to_node.second.GetValue());
        counter++;

        if (counter == finish) break;
        out << ", \n"s;
    }
    out << "\n}"s;
}

void PrintNode(std::ostream& out, const int value) {
    out << value;
}

void PrintNode(std::ostream& out, const double value) {
    out << value;
}

void PrintNode(std::ostream& out, const std::string& str) {
    out << "\""s;
    for (const char& symbol : str) {
        if (symbol == '\"') out << "\\\""s;
        else if (symbol == '\n') out << "\\n"s;
        else if (symbol == '\r') out << "\\r"s;
        else if (symbol == '\t') out << "\t"s;
        else if (symbol == '\\') out << "\\\\"s;
        else out << symbol;
    }
    out << "\""s;
}

void PrintNode(std::ostream& out, const bool value) {
    out << std::boolalpha << value;
}

void Print(const Document& doc, std::ostream& output) {
    visit([&output](const auto& value) { PrintNode(output, value); }, doc.GetRoot().GetValue());
}

}  // namespace json