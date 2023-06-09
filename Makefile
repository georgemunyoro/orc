
CXX = clang++
CXXFLAGS=-g -std=c++11 -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-unused-private-field -Wno-unused-variable -Wno-unused-function -Wno-unused-const-variable
LLVM_FLAGS=`llvm-config --cxxflags --ldflags --system-libs --libs core`

TARGET=orc
SOURCE=main.cpp lexer.cpp utils.cpp parser.cpp ast.cpp orc_llvm.cpp

$(TARGET):
	$(CXX) $(SOURCE) $(CXXFLAGS) $(LLVM_FLAGS) -o $(TARGET)
