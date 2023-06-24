compile:
	llc -filetype=obj hello-world.ll -o hello-world.o
	clang hello-world.o -o hello-world
