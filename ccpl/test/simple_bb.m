int add(int a, int b) {
    return a + b;
}

int x, y, z;
int main() {
    x = 5;
    y = 10;
    
    if (x > 3) {
        z = add(x, y);
    } else {
        z = 0;
    }
    
    output z;
    return 0;
}
