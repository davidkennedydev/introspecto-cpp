Introspecto C++
===

Simple tool to provide reflection/introspection information.

# How it works

1. Build the **inspecto** executable
2. Use `inspecto sample/test.cpp` to parse and generate _constexpr_ representation of classes and structures.
3. Include `instrospecto.h` and write common code that uses the interface

```c++
#include "person.hpp"
#include <introspecto.h>
#include <iostream>

int main() {

  Person person{18, "ze zim"};

  auto personInfo = introspecto::introspect(person);

  personInfo.foreachField(
      [](const std::string_view name, const auto value) {
        std::cout << name << " = " << value << '\n';
      });
}
```

# Dependencies

- C++ STL _(commonly already included in the toolchain/OS)_
- GCC compiler with **C++23** support _(commonly the default compiler)_
- Clang + LLVM with **C++2b** support _(easy to install using any package manager)_
