CXX=g++ -std=c++2b -lLLVM -lclang -lclang-cpp

all: introspecto

introspecto: src/main.cpp
	$(CXX) $^ -o $@

debug: src/main.cpp
	$(CXX) -g $^ -o $@

clean:
	-rm introspecto .*_generated.h

.introspecto_generated.h: introspecto ./sample/main.cpp
	-./$^ 2> $@.log

./sample/print_members: sample/main.cpp | .introspecto_generated.h
	g++ -std=c++23 $< -o $@

test: sample/print_members
	./$<

.PHONY = .test .clean debug
