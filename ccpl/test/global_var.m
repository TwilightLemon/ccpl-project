int global_x;
int global_y;

int add(int a, int b) {
    return a + b;
}

int main() {
    global_x = 5;
    global_y = 10;
    int result;
    result = add(global_x, global_y);
    output result;
    return 0;
}
