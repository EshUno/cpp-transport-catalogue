#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(std::istream& input);

Node LoadBool(istream& input){
    std::string res;
    for (auto i = 0; i < 4; ++i){
        res += input.get();
    }
    if (res == "true") return Node(true);
    res += input.get();
    if (res == "false") return Node(false);
    throw ParsingError("load bool problem");
}

Node LoadNull(istream& input){
    std::string res;
    for (auto i = 0; i < 3; ++i){
        res += input.get();
    }
    if (res != "ull") throw ParsingError("load null problem");
    return Node{nullptr};
}

Node LoadNumber(std::istream& input) {
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
        read_char();
        read_digits();
        is_int = false;
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
                return std::stoi(parsed_num);
            } catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return std::stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

Node LoadString(std::istream& input) {
    using namespace std::literals;

    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    std::string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        } else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                break;
            default:
                // Встретили неизвестную escape-последовательность
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        } else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c && c != ']';) {
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }
    if (!input) {
        throw ParsingError("array load problem"s);
    }
    return Node(std::move(result));
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c && c != '}';) {
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({std::move(key), LoadNode(input)});
    }
    if (!input) {
        throw ParsingError("dict load problem"s);
    }
    return Node(std::move(result));
}

Node LoadNode(istream& input) {
    char c;
    input >> c;

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return LoadString(input);
    } else if (c == 'n'){
        return LoadNull(input);
    } else if (c == 't' || c == 'f'){
        input.putback(c);
        return LoadBool(input);
    }
    else {
        input.putback(c);
        return LoadNumber(input);
    }
}

struct PrintContext {
    std::ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    // Возвращает новый контекст вывода с увеличенным смещением
    PrintContext Indented() const {
        return {out, indent_step, indent_step + indent};
    }
};

void PrintNode(const Node& node, const PrintContext &ctx);

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintString(const std::string &value, std::ostream& out) {
    out.put('"');
    for (const char c : value) {
        switch (c) {
        case '\r':
            out << "\\r";
            break;
        case '\n':
            out << "\\n";
            break;
        case '"':
            [[fallthrough]];
        case '\\':
            out.put('\\');
            [[fallthrough]];
        default:
            out.put(c);
            break;
        }
    }
    out.put('"');
}

template <>
void PrintValue<std::string>(const std::string &value, const PrintContext &ctx) {
    PrintString(value, ctx.out);
}

template <>
void PrintValue<std::nullptr_t>(const std::nullptr_t&, const PrintContext& ctx) {
    ctx.out << "null"sv;
}

template <>
void PrintValue<bool>(const bool& value, const PrintContext &ctx) {
    ctx.out << std::boolalpha << value;
}

template <>
void PrintValue<Array>(const Array& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    bool first = true;
    out << "[\n"sv;
    auto inner_ctx = ctx.Indented();
    for (const Node& node : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put(']');
}

template <>
void PrintValue<Dict>(const Dict& nodes, const PrintContext& ctx) {
    std::ostream& out = ctx.out;
    out << "{\n"sv;
    bool first = true;
    auto inner_ctx = ctx.Indented();
    for (const auto& [key, node] : nodes) {
        if (first) {
            first = false;
        } else {
            out << ",\n"sv;
        }
        inner_ctx.PrintIndent();
        PrintString(key, ctx.out);
        out << ": "sv;
        PrintNode(node, inner_ctx);
    }
    out.put('\n');
    ctx.PrintIndent();
    out.put('}');
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    std::visit(
        [&ctx](const auto& value){PrintValue(value, ctx); },
        node.GetValue());
}

}  // namespace

Node::Node(int value): value_(value){

}
Node::Node(double value): value_(value){

}
Node::Node(std::string value): value_(std::move(value)){

}
Node::Node(bool value): value_(value){

}
Node::Node(Array array): value_(std::move(array)){

}
Node::Node(Dict map): value_(std::move(map)){

}

Node::Node (std::nullptr_t): value_(nullptr) {

}

int Node::AsInt() const{
    if (!IsInt()) throw std::logic_error("wrong type");
    return std::get<int>(value_);
}
bool Node::AsBool() const{
    if (!IsBool()) throw std::logic_error("wrong type");
    return std::get<bool>(value_);
}
double Node::AsDouble() const{
    if (!IsDouble()) throw std::logic_error("wrong type");
    if (IsInt()) return std::get<int>(value_);
    return std::get<double>(value_);
}
const std::string& Node::AsString() const{
    if (!IsString()) throw std::logic_error("wrong type");
    return std::get<std::string>(value_);
}
const Array& Node::AsArray() const{
    if (!IsArray()) throw std::logic_error("wrong type");
    return std::get<Array>(value_);
}
const Dict& Node::AsMap() const{
    if (!IsMap()) throw std::logic_error("wrong type");
    return std::get<Dict>(value_);
}

bool Node::IsInt() const{
    return (holds_alternative<int>(value_));
}
bool Node::IsDouble() const{
    return (holds_alternative<int>(value_) || holds_alternative<double>(value_));
}
bool Node::IsPureDouble() const{
    return (holds_alternative<double>(value_));
}
bool Node::IsBool() const{
    return (holds_alternative<bool>(value_));
}
bool Node::IsString() const{
    return (holds_alternative<std::string>(value_));
}
bool Node::IsNull() const{
    return (holds_alternative<std::nullptr_t>(value_));
}
bool Node::IsArray() const{
    return (holds_alternative<Array>(value_));
}
bool Node::IsMap() const{
    return (holds_alternative<Dict>(value_));
}

bool Node::operator == (const Node &rhs){
    return (value_ == rhs.value_);
}
bool Node::operator == (const Node &rhs) const{
    return (value_ == rhs.value_);
}
bool Node::operator != (const Node &rhs){
    return (value_ != rhs.value_);
}
bool Node::operator != (const Node &rhs) const{
    return (value_ != rhs.value_);
}

//------------Document-------------//

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

bool operator==(const Document& lhs, const Document& rhs) {
    return lhs.GetRoot() == rhs.GetRoot();
}

bool operator!=(const Document& lhs, const Document& rhs) {
    return !(lhs == rhs);
}

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, std::ostream& output) {
    PrintNode(doc.GetRoot(), PrintContext{output});
}

}  // namespace json
