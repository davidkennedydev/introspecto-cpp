
// XXX: Do not change include order, todo: fix it with modules
// clang-format off
#include "person.hpp"
#include "../include//introspecto.h"
#include "./.introspecto_generated.h"
// clang-format on

#include <iostream>

int main() {

  Person person{18, "ze zim"};

  auto personInfo = introspecto::introspect(person);

  auto printFieldNameValue = [](const std::string_view name, auto &value) {
    std::cout << name << " = " << value << '\n';
  };

  personInfo.foreachField(printFieldNameValue);

  std::cout << "\n";

  AnotherKindOfPerson peculiarPerson{{10, "fulano"}, "smart"};

  introspecto::introspect<Person>(peculiarPerson)
      .foreachField(printFieldNameValue);
  introspecto::introspect(peculiarPerson).foreachField(printFieldNameValue);
}
