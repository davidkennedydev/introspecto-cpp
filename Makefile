CXX=g++ -std=c++2b -lLLVM -lclang -lclang-cpp

all: introspecto

introspecto: src/main.cpp
	$(CXX) $^ -o $@

reflection_generated.cpp: introspecto src/sample/test.cpp
	./$^

clean:
	-rm introspecto
