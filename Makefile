compile:
	llc -filetype=obj hello-world.ll -o hello-world.o
	clang -no-pie hello-world.o -o hello-world
