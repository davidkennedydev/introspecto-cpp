#pragma once

#include <string_view>

namespace introspecto {

template <typename Callable, typename FieldType>
concept VisitWithNameAnd =
    requires(Callable visit, std::string_view name, FieldType value) {
      { visit(name, value) } -> std::same_as<void>;
    };


template <typename Callable, typename... FieldType>
concept Visitor = (... && VisitWithNameAnd<Callable, FieldType>);

template <typename T> class Introspect {
public:
  constexpr void foreachField(Visitor auto &&apply);
  constexpr void foreachBaseClass(Visitor auto &&apply);
};

template <typename UserType>
Introspect<UserType> introspect(UserType &instance) {
  return Introspect<UserType>(instance);
}

}; // namespace introspecto
