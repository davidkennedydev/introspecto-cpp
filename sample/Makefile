CXX=g++ -std=c++23

INTROSPECTO_PATH=./..
INCLUDE=-isystem ${INTROSPECTO_PATH}/include 

all: print_members

.introspecto_generated.h: ${INTROSPECTO_PATH}/introspecto main.cpp
	-./$^ 2> $@.log

print_members: main.cpp | .introspecto_generated.h
	${CXX} $< -o $@

test: print_members
	./$<

clean:
	-rm -f .*_generated.h* print_members

.PHONY: clean
