#include <string>

struct Person {
  int age;
  std::string fullname;
};


class AnotherKindOfPerson : Person {
  std::string peculiarity;
};
