
func add(a int, b int) int {
    if ((a + b) > 100) {
        return(100);
    } else {
        return(a + b);
    }
}

func main() void {
    var addition int = add(9393, 2);
    printf("%d\n", addition);
}
