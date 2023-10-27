#pragma once

#include <string>

struct Person {
  int age;
  std::string fullname;
};

class AnotherKindOfPerson : Person {
public:
  std::string peculiarity;
};
