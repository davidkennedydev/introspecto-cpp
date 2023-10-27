CXX=g++ -std=c++2b -lLLVM -lclang -lclang-cpp

all: introspecto

introspecto: src/main.cpp
	$(CXX) $^ -o $@

clean:
	-rm introspecto .*_generated.h

.introspecto_generated.h: introspecto ./sample/person.hpp
	-./$^ 2> $@.log

./sample/print_members: sample/main.cpp | .introspecto_generated.h
	g++ -std=c++23 $< -o $@

test: sample/print_members
	./$<

.PHONY = .test .clean

