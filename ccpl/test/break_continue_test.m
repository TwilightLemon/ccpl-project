int find_first_positive(int n) {
    int i;
    i = 0;
    while (i < n) {
        input i;
        if (i > 0) {
            return i;
        }
        i = i + 1;
    }
    return 0;
}

int sum_even(int n) {
    int sum, i;
    sum = 0;
    i = 0;
    for (i = 0; i < n; i = i + 1) {
        if (i == 10) {
            break;
        }
        if (i - (i / 2) * 2 != 0) {
            continue;
        }
        sum = sum + i;
    }
    return sum;
}

int nested_loops() {
    int i, j, found;
    found = 0;
    i = 0;
    while (i < 10) {
        j = 0;
        while (j < 10) {
            if (i * j == 42) {
                found = 1;
                break;
            }
            j = j + 1;
        }
        if (found == 1) {
            break;
        }
        i = i + 1;
    }
    return found;
}

int main() {
    int result;
    result = sum_even(20);
    output result;
    return 0;
}
