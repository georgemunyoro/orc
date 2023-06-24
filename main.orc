
func say_something(something string) void {
    printf("%s\n", something);
}

func main() void {
    var age int = 33;
    
    if (age < 200) {
        say_something("IT IS OVER 200");
        if (age < 100) {
            say_something("SOMETHING");
        } else {
            say_something("OTHER");
        }
    } else {
        say_something("WHATHHATHAT");
    }
}
