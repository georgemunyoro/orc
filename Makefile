all:
	g++ -std=c++11 -o orc main.cpp lexer.cpp utils.cpp parser.cpp ast.cpp -lpthread