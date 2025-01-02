#pragma once

#include <string>

struct Person {
  int age;
  std::string fullname;
};

class AnotherKindOfPerson : public Person {
public:
  std::string peculiarity;
};
