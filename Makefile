compile:
	llc -filetype=obj out.ll -o out.o
	clang -no-pie out.o -o hello-world
