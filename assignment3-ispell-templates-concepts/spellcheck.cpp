#include "spellcheck.h"

#include <algorithm>
#include <iostream>
#include <numeric>
#include <ranges>
#include <set>
#include <vector>
#include <iterator>
#include <concepts>

template <std::input_iterator Iterator, std::indirect_unary_predicate<Iterator> UnaryPred>
std::vector<Iterator> find_all(Iterator begin, Iterator end, UnaryPred pred);

Corpus tokenize(std::string& source) {
  std::vector<std::string::iterator> its = find_all(source.begin(), source.end(), [](char c) { return std::isspace(c); });
  // std::vector<std::string::iterator> its = find_all(source.begin(), source.end(), 42);
  Corpus corpus;
  auto ins = std::inserter(corpus, corpus.end());
  std::transform(its.begin(), its.end() - 1, its.begin() + 1, ins,
                 [&source](auto left, auto right) { return Token(source, left, right); });
  std::erase_if(corpus, [](const Token& t) { return t.content.empty(); });
  return corpus;
}

std::set<Misspelling> spellcheck(const Corpus& source, const Dictionary& dictionary) {
  auto misspelled = source | std::ranges::views::filter([&dictionary](const Token& t) { return dictionary.find(t.content) == dictionary.end(); }) | std::ranges::views::transform([&dictionary](const Token& t) { auto view=(dictionary | std::ranges::views::filter([&t](auto it_d){ return levenshtein(it_d, t.content) < 2; })); return Misspelling(t, std::set<std::string>(view.begin(), view.end())); });
  auto correctable = misspelled | std::ranges::views::filter([](const Misspelling& m) { return !m.suggestions.empty(); });
  return std::set<Misspelling>(correctable.begin(), correctable.end());
  
  // return std::set<Misspelling>();
};

/* Helper methods */

#include "utils.cpp"