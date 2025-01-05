// XXX: Do not change include order, todo: fix it with modules
// clang-format off
#include "person.hpp"
#include "../include//introspecto.h"
#include "./.introspecto_generated.h"
// clang-format on

#include <iostream>
#include <string_view>

int main() {

  Person person{18, "ze zim"};

  auto person_info = introspecto::introspect(person);

  auto printFieldNameValue = [](const std::string_view name, auto &value) {
    std::cout << name << " = " << value << '\n';
  };

  person_info.foreachField(printFieldNameValue);

  std::cout << "\n";

  AnotherKindOfPerson peculiar_person{{10, "fulano"}, "smart"};

  auto another_person_info = introspecto::introspect(peculiar_person);

  another_person_info.foreachBaseClass(
      [&](const std::string_view name, auto& instance) {
        auto base_instance_info = introspecto::introspect(instance);
        base_instance_info.foreachField(printFieldNameValue);
      });
  another_person_info.foreachField(printFieldNameValue);
}
