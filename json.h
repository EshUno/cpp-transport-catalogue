#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <string_view>
namespace json {

class Node;
// Сохраните объявления Dict и Array без изменения
using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

class Node{
public:
    /* Реализуйте Node, используя std::variant */
    using Value = std::variant<std::nullptr_t, int, double, std::string, bool, Array, Dict>;
    Node() = default;
    Node(int value);
    Node(double value);
    Node(std::string value);
    Node(bool value);
    Node(Array array);
    Node(Dict map);
    Node (std::nullptr_t value);
    int AsInt() const;
    bool AsBool() const;
    double AsDouble() const;
    const std::string& AsString() const;
    const Array& AsArray() const;
    const Dict& AsMap() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsBool() const;
    bool IsString() const;
    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    const Value &GetValue() const {return value_;}

    bool operator == (const Node &rhs);
    bool operator == (const Node &rhs) const;
    bool operator != (const Node &rhs);
    bool operator != (const Node &rhs) const;
private:
    Value value_;
};

class Document {
public:
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

bool operator==(const Document& lhs, const Document& rhs);

bool operator!=(const Document& lhs, const Document& rhs);

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
