struct Student {
    int id;
    int score;
};

int max(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

main() {
    struct Student s;
    int result;
    
    input s.id;
    input s.score;
    
    result = max(s.id, s.score);
    output "Maximum: ";
    output result;
    output "\n";
    
    return 0;
}