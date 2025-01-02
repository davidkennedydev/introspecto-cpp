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
concept Visitor = (... && VisitWithNameAnd<Callable, FieldType>);

template <typename T> class Introspect {
public:
  constexpr void foreachField(Visitor auto &&apply);
  constexpr void foreachBaseClass(Visitor auto &&apply);
};

template <typename UserType>
Introspect<UserType> introspecto(UserType &instance) {
  return Introspect<UserType>(instance);
}

// User code

struct Person {
  int age;
  std::string fullname;
};

struct PersonWithExtra : public Person {
  double extra;
};

// Generated

template <> class Introspect<Person> {
  Person &instance;

public:
  constexpr Introspect(Person &instance) : instance(instance) {}

  constexpr void foreachField(Visitor auto &&apply) {
    apply("age", instance.age);
    apply("fullname", instance.fullname);
  }

  constexpr void foreachBaseClass(Visitor auto &&apply) {}
};

template <> class Introspect<PersonWithExtra> {
  PersonWithExtra &instance;

public:
  constexpr Introspect(PersonWithExtra &instance) : instance(instance) {}

  constexpr void foreachField(Visitor auto &&apply) {
    apply("extra", instance.extra);
  }

  constexpr void foreachBaseClass(Visitor auto &&apply) {
    apply("Person", static_cast<Person>(instance));
  }
};

// Use generated

#include <iostream>

int main() {

  Person person{18, "ze zim"};

  auto person_info = introspecto(person);

  person_info.foreachField([](const std::string_view name, const auto value) {
    std::cout << value << std::endl;
  });

  std::cout << "---" << std::endl;

  PersonWithExtra person_with_extra{10, "fulano", 42.00};

  auto person_info_with_extra = introspecto(person_with_extra);

  person_info_with_extra.foreachBaseClass(
      [](const std::string_view name, auto base_instance) {
        std::cout << name << ":\n";

        auto base_instance_info = introspecto(base_instance);

        base_instance_info.foreachField(
            [](const std::string_view name, auto value) {
              std::cout << "  " << value << std::endl;
            });
      });

  person_info_with_extra.foreachField(
      [](const std::string_view name, const auto value) {
        std::cout << value << std::endl;
      });
}
