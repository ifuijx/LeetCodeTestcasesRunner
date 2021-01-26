#pragma once

#ifndef __IFUIJX_PARSER__
#define __IFUIJX_PARSER__

#include <string>
#include <vector>
#include <set>
#include <any>
#include <fstream>
#include <cassert>
#include <type_traits>

class Context {
public:
    bool isempty(char c) { return c == ' '; }

public:
    Context(std::string const & input) : _input(input), _idx(0) { }

    char consume() {
        bool a = isend();
        assert(!isend());
        return _input[_idx++];
    }

    char current() const { return isend() ? '\0' : _input[_idx]; }

    char next() const { return _idx + 1 < _input.size() ? _input[_idx + 1] : '\0'; }

    bool isend() const { return _idx >= _input.size(); }

    void remove_empty() { for (; !isend() && isempty(current()); ++_idx); }

private:
    std::string const _input;
    int _idx;
};

template <typename TNumber>
std::any parse_value(Context &);

template <typename TNumber>
std::vector<std::any> parse_values(Context & context) {
    context.remove_empty();
    assert(context.current() == '[');
    context.consume();

    std::vector<std::any> values;
    while (true) {
        context.remove_empty();
        if (context.current() == ']') break;
        assert(!context.isend());
        values.push_back(parse_value<TNumber>(context));

        context.remove_empty();
        if (context.current() == ',') context.consume();
    }

    context.consume();

    return values;
}

std::string parse_string(Context & context) {
    context.remove_empty();
    assert(context.current() == '"');
    context.consume();

    std::string str;
    while (context.current() != '"') {
        str.push_back(context.consume());
    }
    context.consume();
    return str;
}

template <typename T>
T parse_number(Context & context) {
    assert(false);
}

template <>
int parse_number<int>(Context & context) {
    context.remove_empty();
    std::string str;
    if (context.current() == '+' || context.current() == '-') str.push_back(context.consume());
    assert(context.current() >= '0' && context.current() <= '9');
    for (; context.current() >= '0' && context.current() <= '9'; str.push_back(context.consume()));
    return std::stoi(str);
}

template <>
double parse_number<double>(Context & context) {
    context.remove_empty();
    std::string str;
    // Simple validation
    std::set<char> validchars {'+', '-', '.', 'e', 'E'};
    for (; (context.current() >= '0' && context.current() <= '9') || (validchars.find(context.current()) != validchars.end()); context.consume());
    assert(str.size());
    return std::stod(str);
}

template<>
float parse_number<float>(Context & context) {
    context.remove_empty();
    std::string str;
    // Simple validation
    std::set<char> validchars {'+', '-', '.', 'e', 'E'};
    for (; (context.current() >= '0' && context.current() <= '9') || (validchars.find(context.current()) != validchars.end()); context.consume());
    assert(str.size());
    return std::stof(str);
}

template <typename TNumber>
std::any parse_value(Context & context) {
    context.remove_empty();
    if (context.current() == '[') return parse_values<TNumber>(context);
    else if (context.current() == '"') return parse_string(context);
    else if (context.current() == '+' || context.current() == '-' || (context.current() >= '0' && context.current() <= '9'))
        return parse_number<TNumber>(context);
    return std::any();
}

template <typename TNumber>
std::any parse_value(std::string const & str) {
    Context context(str);
    context.remove_empty();
    if (context.current() == '[') return parse_values<TNumber>(context);
    else if (context.current() == '"') return parse_string(context);
    else if (context.current() == '+' || context.current() == '-' || (context.current() >= '0' && context.current() <= '9'))
        return parse_number<TNumber>(context);
    return std::any();
}

template <typename TNumber = int>
std::vector<std::any> parse_params(std::string const & filename) {
    std::ifstream in(filename);

    std::vector<std::any> params;
    for (std::string line; std::getline(in, line); ) {
        params.push_back(parse_value<TNumber>(line));
    }

    return params;
}

#endif /* !__IFUIJX_PARSER__ */
