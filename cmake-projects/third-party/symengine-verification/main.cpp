#include <iostream>
#include <symengine/add.h>
#include <symengine/expression.h>
#include <symengine/integer.h>
#include <symengine/symbol.h>

using SymEngine::add;
using SymEngine::Expression;
using SymEngine::Integer;
using SymEngine::integer;
using SymEngine::Symbol;
using SymEngine::symbol;

int main() {
  std::cout << "SymEngine Verification" << std::endl;

  auto x = symbol("x");
  auto y = symbol("y");
  auto expr = add(x, y);

  std::cout << "Expression: " << *expr << std::endl;

  auto i1 = integer(10);
  auto i2 = integer(20);
  auto sum = add(i1, i2);

  std::cout << "Sum: " << *sum << std::endl;

  if (sum->__str__() == "30") {
    std::cout << "Verification Passed!" << std::endl;
    return 0;
  } else {
    std::cout << "Verification Failed!" << std::endl;
    return 1;
  }
}
