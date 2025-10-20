int x, y;

int fibonacci(int n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

int factorial(int n) {
    int result;
    result = 1;
    while (n > 0) {
        result = result * n;
        n = n - 1;
    }
    return result;
}

int sum_array(int n) {
    int sum, i;
    sum = 0;
    i = 0;
    while (i < n) {
        sum = sum + i;
        i = i + 1;
    }
    return sum;
}

void test_comparisons(int a, int b) {
    int eq, ne, lt, le, gt, ge;
    eq = (a == b);
    ne = (a != b);
    lt = (a < b);
    le = (a <= b);
    gt = (a > b);
    ge = (a >= b);
    
    if (eq) {
        output "Equal\n";
    } else {
        output "Not equal\n";
    }
    
    return;
}

int main() {
    int n, result;
    
    output "Enter a number: ";
    input n;
    
    result = fibonacci(n);
    output "Fibonacci: ";
    output result;
    output "\n";
    
    result = factorial(n);
    output "Factorial: ";
    output result;
    output "\n";
    
    result = sum_array(n);
    output "Sum 0 to n: ";
    output result;
    output "\n";
    
    test_comparisons(n, 5);
    
    return 0;
}
