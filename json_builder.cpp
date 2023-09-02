#include "json_builder.h"

using namespace json;
using namespace std::literals;

Builder::Builder()
{
    nodes_stack_.emplace_back(&root_);
}

KeyNodeProxy Builder::Key(std::string key)
{
    auto parent = GetTop();
    if (parent == nullptr || !parent->IsDict())
    {
        throw std::logic_error("Key() outside of a dictionary");
    }

    auto& parentDict = const_cast<json::Dict&>(parent->AsDict());
    auto& addedValueForKey = parentDict[key];
    nodes_stack_.emplace_back(&addedValueForKey);

    return *this;
}

Builder& Builder::Value(json::Node value)
{
    auto parent = GetTop();

    if (parent == nullptr || (!parent->IsNull() && !parent->IsArray()))
    {
        throw std::logic_error("Value(): parent == nullptr or parent is not empty or array");
    }

    if (parent->IsArray())
    {
        const_cast<json::Array&>(parent->AsArray()).emplace_back(value);
    }
    else
    {
        *nodes_stack_.back() = value;
        nodes_stack_.pop_back();
    }
    return *this;
}

DictNodeProxy Builder::StartDict()
{
    auto parent = GetTop();

    if (parent == nullptr || (!parent->IsNull() && !parent->IsArray()))
    {
        throw std::logic_error("StartDict(): parent == nullptr or parent is not empty or array");
    }

    if (parent->IsArray())
    {
        const_cast<json::Array&>(parent->AsArray()).push_back(json::Dict{});
        nodes_stack_.emplace_back(&const_cast<json::Array&>(parent->AsArray()).back());
    }
    else
    {
        *nodes_stack_.back() = json::Dict{};
    }
    return *this;
}

Builder& Builder::EndDict()
{
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict())
    {
        throw std::logic_error("EndDict(): outside of a dictionary");
    }
    nodes_stack_.pop_back();
    return *this;
}

ArrayNodeProxy Builder::StartArray()
{
    auto parent = GetTop();
    if (parent == nullptr || (!parent->IsNull() && !parent->IsArray()))
    {
        throw std::logic_error("StartArray(): parent == nullptr or parent is not empty or array");
    }

    if (parent->IsArray())
    {
        const_cast<json::Array&>(parent->AsArray()).push_back(json::Array{});
        nodes_stack_.emplace_back(&const_cast<json::Array&>(parent->AsArray()).back());
    }
    else
    {
        *nodes_stack_.back() = json::Array{};
    }

    return *this;
}

Builder& Builder::EndArray()
{
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray())
    {
        throw std::logic_error("EndArray(): outside of an array");
    }
    nodes_stack_.pop_back();
    return *this;
}


json::Node Builder::Build()
{
    if (root_.IsNull() || !nodes_stack_.empty())
    {
        throw std::logic_error("Build(): object is not ready"s);
    }
    return root_;
}

Node* Builder::GetTop()
{
    if (!nodes_stack_.empty())
    {
        return nodes_stack_.back();
    }
    return nullptr;
}

// Proxies:
KeyNodeProxy NodeProxy::Key(std::string key) {
    return builder_.Key(std::move(key));
}

Builder& NodeProxy::Value(Node value) {
    return builder_.Value(std::move(value));
}

DictNodeProxy NodeProxy::StartDict() {
    return builder_.StartDict();
}

ArrayNodeProxy NodeProxy::StartArray() {
    return builder_.StartArray();
}

Builder& NodeProxy::EndDict() {
    return builder_.EndDict();
}

Builder& NodeProxy::EndArray() {
    return builder_.EndArray();
}

ArrayNodeProxy ArrayNodeProxy::Value(Node value)
{
    return NodeProxy::Value(std::move(value));
}
