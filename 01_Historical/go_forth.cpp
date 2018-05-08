/*
Go forth
- Existence of a data stack.
- Existence of a data heap.
- Abstractions in the form of procedures.

Having learned to avoid modeling actual hardware and language limitations that
the chapter is based on, this time I create C++ abstractions that let the final
code use techniques similar to those used in the Python implementation.
*/

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <algorithm>
#include <vector>
#include <iterator>
#include <sstream>

using namespace std;

// C++ abstractions to make this implementation more like the Python code.
namespace {
    template<typename T>
    void print(T const& value) {
        cout << value << '\n';
    }

    class object {
    public:
        object() = default;

        template<typename T>
        explicit object(T value)
                : storage(new model<T>(value)) {
        }

        template<typename T>
        T get() const {
            return storage->get<T>();
        }

        template<typename T>
        T& peek() {
            return storage->peek<T>();
        }

    private:
        struct concept {
            virtual ~concept() = default;

            virtual concept* copy() const = 0;

            template<typename T>
            T get() const {
                model <T> const* typed = static_cast<model <T> const*>((this));
                return typed->get_();
            }

            template<typename T>
            T& peek() {
                model <T>* typed = static_cast<model <T>*>(this);
                return typed->peek_();
            }
        };


        template<typename T>
        struct model : public concept {

            explicit model(T value)
                    : value(move(value)) {
            }

            concept* copy() const override {
                return new model<T>(value);
            }

            T get_() const {
                return value;
            }

            T& peek_() {
                return value;
            }

            T value;
        };

        unique_ptr<concept> storage;
    };
}


stack<object> operands;
map<std::string, object> free_store;


template<typename T>
void push(T&& value) {
    operands.emplace(std::forward<T&&>(value));
}


template<typename T>
T& peek() {
    return operands.top().peek<T>();
}


template<typename T>
T pop() {
    auto value = operands.top().get<T>();
    operands.pop();
    return value;
}


template<typename T>
void associate(string const& name, T&& value) {
    free_store.emplace(name, std::forward<T&&>(value));
}


template<typename T>
T& lookup(string const& name) {
    return free_store[name].peek<T>();
}


/**
Reads a file path from the stack.
Places the contents of that file on the stack as a single object.
*/
void read_file() {
    ifstream file(pop<string>());
    string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
    push(content);
}


/**
Replaces the string at the top of the stack with a version with all
non-alphanumeric characters replaced with whitespace.
 */
void filter_chars() {
    string content = pop<string>();
    string transformed;
    auto filter = [](char c) {
        if (isalnum(c)) {
            return static_cast<char>(tolower(c));
        }
        else {
            return static_cast<char>(' ');
        }
    };
    transform(content.begin(), content.end(), back_inserter(transformed), filter);
    push(transformed);
}


/**
Read a string from the stack.
Place list of words back on stack.
*/
void scan() {
    istringstream content(pop<string>());
    std::vector<std::string> words;
    copy(istream_iterator<string>(content), istream_iterator<string>(), back_inserter(words));
    push(words);
}


/**
Takes a list of words on the stack and removes the stop words.
Push each individual word on to the stack.
*/
void remove_stop_words() {
    push(string("stop_words.txt"));
    read_file();
    filter_chars();
    scan();
    for (char c = 'a'; c < 'z'; ++c) {
        peek<vector<string>>().push_back(string(1, c));
    }
    peek<vector<string>>().push_back(""); // Why do we still get a frequency for empty string?

    associate("stop_words", pop<vector<string>>());
    associate("words", vector<string>());

    for (auto& word : pop<vector<string>>()) {
        auto it = find(begin(lookup<vector<string>>("stop_words")), end(lookup<vector<string>>("stop_words")), word);
        if (it == end(lookup<vector<string>>("stop_words"))) {
            lookup<vector<string>>("words").push_back(word);
        }
    }

    //copy(begin(lookup<vector<string>>("words")), end(lookup<vector<string>>("words")), back_inserter(operands));
    for (auto& word : lookup<vector<string>>("words")) {
        push(word);
    }
}


/**
*/
void frequencies() {
    associate("word_freqs", map<string, int>());
    while (!operands.empty()) {
        auto it = lookup<map<string, int>>("word_freqs").find(peek<string>());
        auto itEnd = lookup<map<string, int>>("word_freqs").end();
        if (it != itEnd) {
            push(it->second);
            push(1);
            push(pop<int>() + pop<int>());
        } else {
            push(1);
        }
        auto freq = pop<int>();
        lookup<map<string, int>>("word_freqs")[pop<string>()] = freq;
    }

    push(lookup<map<string, int>>("word_freqs"));
}


void sort() {
    map<string, int> freqs_map = pop<map<string, int>>();
    vector<std::pair<string, int>> freqs_list;
    copy(begin(freqs_map), end(freqs_map), back_inserter(freqs_list));
    sort(begin(freqs_list), end(freqs_list), [](pair<string, int> const& lhs, pair<string, int> rhs) {
        return lhs.second < rhs.second;
    });

    for (auto& freq : freqs_list) {
        push<pair<string, int>>(std::move(freq));
    }
}


int main() {
    operands.emplace("SENTINEL");
    push(string("pride-and-prejudice .txt"));
    read_file();
    filter_chars();
    scan();
    remove_stop_words();
    frequencies();
    sort();

    for (int i = 0; i < 25 && operands.size() > 1; ++i) {
        std::pair<string, int> freq = pop<std::pair<string, int>>();
        std::cout << "'" << freq.first << "': " << freq.second << '\n';
    }
}
