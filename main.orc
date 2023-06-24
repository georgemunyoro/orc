
func say_hello(something string) void {
    printf("Hello, %s!", something);
}

func main() void {
    var old_age int = 33;
    var age int = 18;
    age = old_age;
    printf("age is %d", age);
    say_hello("George");
}
