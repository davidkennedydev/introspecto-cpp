#pragma once

#include <string_view>

namespace introspecto {

template <typename Callable, typename FieldType>
concept VisitWithNameAnd =
    requires(Callable visit, std::string_view name, FieldType value) {
      { visit(name, value) } -> std::same_as<void>;
    };

template <typename Callable, typename... FieldType>
concept FieldVisitor = (... && VisitWithNameAnd<Callable, FieldType>);

template <typename T> class Introspect {
public:
  constexpr void foreachField(FieldVisitor auto &&apply);
};

template <typename UserType>
Introspect<UserType> introspect(UserType &instance) {
  return Introspect<UserType>(instance);
}

}; // namespace introspecto

#include "../.introspecto_generated.h"
