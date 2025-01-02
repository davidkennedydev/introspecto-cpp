
// XXX: Do not change include order, todo: fix it with modules
#include "person.hpp"
#include "../include//introspecto.h"
#include "./.introspecto_generated.h"

#include <iostream>

int main() {

  Person person{18, "ze zim"};

  auto personInfo = introspecto::introspect(person);

  personInfo.foreachField([](const std::string_view name, auto& value) {
    std::cout << name << " = " << value << '\n';
  });
}
