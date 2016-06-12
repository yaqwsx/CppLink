#pragma once
#include <iostream>
#include <cassert>
#include <type_traits>
#include <string>

#ifndef _CPPLINK_EMBEDDED_CODE_
    #include "maybe.h"        
#endif // !_CPPLINK_EMBEDDED_CODE_

namespace cpplink {

template <typename...> struct all_string;
template <> struct all_string<> : std::true_type {};
template <typename T, typename... Tail> struct all_string<T, Tail...>
    : std::integral_constant<bool, std::is_convertible<T, std::string>::value && all_string<Tail...>::value> {};

struct CsvDialect {
    static constexpr const char* header    = "";
    static constexpr const char* separator = ",";
    static constexpr const char* line_end  = "\n";
    static constexpr bool        quote_string = true;
};

struct ExcelCsvDialect {
    static constexpr const char* header    = "sep=,\n";
    static constexpr const char* separator = ",";
    static constexpr const char* line_end  = "\n";
    static constexpr bool        quote_string = false;
};

struct PlainTextDialect {
    static constexpr const char* header    = "";
    static constexpr const char* separator = "\t";
    static constexpr const char* line_end  = "\n";
    static constexpr bool        quote_string = false;
};

struct ItemEscaper {
    static std::string escape_string(const std::string& s) {
        std::string res;
        res.reserve(s.size() * 6 / 5);
        for (char c : s) {
            if (c == '"')
                res.push_back('\\');
            res.push_back(c);
        }
        return res;
    }
};

template <typename Dialect, typename T>
struct ItemWriter : ItemEscaper {
    static void write_item(std::ostream& file, const T& t) {
        file << t;
    }
};

template <typename Dialect, typename T>
struct ItemWriter<Dialect, Maybe<T>>: ItemEscaper {
    static void write_item(std::ostream& file, const Maybe<T>& t) {
        if (!t.isValid())
            ItemWriter<Dialect, std::string>::write_item(file, "None");
        else
            ItemWriter<Dialect, T>::write_item(file, t.value);
    }
};

template <typename Dialect>
struct ItemWriter<Dialect, std::string> : ItemEscaper {
    static void write_item(std::ostream& file, const std::string& t) {
        file << '"' << escape_string(t) << '"';
    }
};

template <typename Dialect, typename... Columns>
class TableWriter {
private:
    static const constexpr size_t arg_count = sizeof...(Columns);
public:
    /*constexpr*/ TableWriter(std::ostream& o, std::initializer_list<std::string> columns) 
        : _file(o)
    {
        // columns.size() is not a constexpr... yet
        //static_assert(columns.size() == arg_count, "Wrong number of columns");
        assert(columns.size() == arg_count && "Wrong number of columns");
        _file << Dialect::header;

        bool first = true;
        for (const std::string& name : columns) {
            if (!first)
                _file << Dialect::separator;
            ItemWriter<Dialect, std::string>::write_item(_file, name);
            first = false;
        }
        _file << Dialect::line_end;
    }

    void write_line(Columns... columns) {
        write(columns...);
    }

private:
    template <typename T>
    void write(T t) {
        ItemWriter<Dialect, T>::write_item(_file, t);
        _file << Dialect::line_end;
    }

    template <typename T, typename... Args>
    void write(T t, Args... args) {
        ItemWriter<Dialect, T>::write_item(_file, t);
        _file << Dialect::separator;
        write(args...);
    }

    template <typename T>
    void write_item(const T& t);

    std::ostream& _file;
};

};
