CXX=g++ -std=c++26 -lLLVM -lclang -lclang-cpp -Wsuggest-override

all: introspecto

introspecto: src/main.cpp
	$(CXX) $^ -o $@

debug: src/main.cpp
	$(CXX) -g $^ -o $@

.introspecto_generated.h: introspecto ./sample/main.cpp
	-./$^ 2> $@.log

./sample/print_members: sample/main.cpp | .introspecto_generated.h
	g++ -std=c++23 -I./include $< -o $@

test: sample/print_members
	./$<

clean:
	-rm -f introspecto .*_generated.h ./sample/print_members

.PHONY = test clean debug
