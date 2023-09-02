#pragma once
#include "json.h"

namespace json {

struct NodeProxy;
struct KeyNodeProxy;
struct ValueNodeProxy;
struct ArrayNodeProxy;
struct DictNodeProxy;

class Builder{
public:
    Builder();
    KeyNodeProxy Key(std::string key);
    Builder& Value(json::Node value);
    DictNodeProxy StartDict();
    Builder& EndDict();
    ArrayNodeProxy StartArray();
    Builder& EndArray();
    json::Node Build();

private:
    Node* GetTop();

private:
    json::Node root_;
    std::vector<Node*> nodes_stack_;
};

struct NodeProxy
{
public:
    NodeProxy(Builder& builder) : builder_{builder} {}
    KeyNodeProxy Key(std::string str);
    Builder& Value(json::Node valueNode);
    ArrayNodeProxy StartArray();
    DictNodeProxy StartDict();
    Builder& EndArray();
    Builder& EndDict();

private:
    Builder& builder_;
};

struct KeyNodeProxy : public NodeProxy
{
public:
    KeyNodeProxy(Builder& builder) : NodeProxy(builder) {}
    ValueNodeProxy Value(Node value);
    // Forbid:
    KeyNodeProxy Key(std::string key) = delete;
    Builder& EndDict() = delete;
    Builder& EndArray() = delete;
};

struct ValueNodeProxy : public NodeProxy
{
    ValueNodeProxy(Builder& builder) : NodeProxy(builder) {}
    // Forbid:
    Builder& Value(Node value) = delete;
    DictNodeProxy StartDict() = delete;
    ArrayNodeProxy StartArray() = delete;
    Builder& EndArray() = delete;
};

struct DictNodeProxy : public NodeProxy
{
    DictNodeProxy(Builder& builder) : NodeProxy(builder) {}
    // Forbid:
    Builder& Value(Node value) = delete;
    DictNodeProxy StartDict() = delete;
    ArrayNodeProxy StartArray() = delete;
    Builder& EndArray() = delete;
};

struct ArrayNodeProxy : public NodeProxy
{
    ArrayNodeProxy(Builder& b) : NodeProxy(b) {}
    ArrayNodeProxy Value(json::Node value);
    // Forbid:
    KeyNodeProxy Key(std::string key) = delete;
    Builder& EndDict() = delete;
};

} // namespace json
