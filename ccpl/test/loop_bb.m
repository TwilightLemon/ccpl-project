int sum(int n) {
    int result, i;
    result = 0;
    i = 0;
    while (i < n) {
        result = result + i;
        i = i + 1;
    }
    return result;
}

int main() {
    int n;
    input n;
    output sum(n);
    return 0;
}
