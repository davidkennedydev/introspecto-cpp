#include <string>
#include <string_view>

// Constexpr introspection interface
//

template <typename Callable, typename FieldType>
concept VisitWithNameAnd =
    requires(Callable visit, std::string_view name, FieldType value) {
      { visit(name, value) } -> std::same_as<void>;
    };

template <typename Callable, typename... FieldType>
concept FieldVisitor =
    (... && VisitWithNameAnd<Callable, FieldType>);

template <typename T> class Introspect {
public:
  constexpr void foreachField(FieldVisitor auto &&apply);
};

template <typename UserType>
Introspect<UserType> introspecto(UserType& instance) {
  return Introspect<UserType>(instance);
}

// User code

struct Person {
  int age;
  std::string fullname;
};

// Generated

template <> class Introspect<Person> {
  Person &instance;

public:
  constexpr Introspect(Person &instance) : instance(instance) {}

  constexpr void foreachField(FieldVisitor auto &&apply) {
    apply("age", instance.age);
    apply("fullname", instance.fullname);
  }
};

// Use generated

#include <iostream>

int main() {

  Person person{18, "ze zim"};

  auto personInfo = introspecto(person);

  personInfo.foreachField(
      [](const std::string_view name, const auto value) {
        std::cout << value << std::endl;
      });
}
