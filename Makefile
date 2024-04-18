cc: cc.cpp c.tab.cpp c.lex.cpp codegen.cpp codegen.hpp scope.cpp scope.hpp
	g++ c.tab.cpp c.lex.cpp cc.cpp -lm -ll `llvm-config --cxxflags --libs --ldflags --system-libs` -o $@

c.tab.cpp c.tab.hpp: c.y
	bison -o c.tab.cpp -d c.y

c.lex.cpp: c.l c.tab.hpp
	flex -o c.lex.cpp -l c.l

clean::
	rm -f c.tab.cpp c.tab.hpp c.lex.cpp cc c.output

codegen: codegen.cpp
	g++ codegen.cpp -lm -ll `llvm-config --cxxflags --libs --ldflags --system-libs` -o $@ 

testllvm: test.cpp
	g++ test.cpp -lm -ll `llvm-config --cxxflags --libs --ldflags --system-libs` -o $@ 