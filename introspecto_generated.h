template <> class Introspect<Person> {
  Person &instance;
public:

  constexpr Introspect(Person &instance): instance(instance) {}

  constexpr void foreachField(FieldVisitor auto &&apply) {
    apply("age", instance.age);
    apply("fullname", instance.fullname);
  }
};

template <> class Introspect<AnotherKindOfPerson> {
  AnotherKindOfPerson &instance;
public:

  constexpr Introspect(AnotherKindOfPerson &instance): instance(instance) {}

  constexpr void foreachField(FieldVisitor auto &&apply) {
    apply("peculiarity", instance.peculiarity);
  }
};

