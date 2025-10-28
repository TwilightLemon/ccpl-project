struct Vector {
    int x;
    int y;
    int z;
};

struct Vector add(struct Vector a, struct Vector b) {
    struct Vector result;
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

int main() {
    struct Vector a;
    struct Vector b;
    
    a.x = 1;
    a.y = 2;
    a.z = 3;
    
    b.x = 4;
    b.y = 5;
    b.z = 6;
    
    output a.x;
    output a.y;
    output a.z;
    
    output b.x;
    output b.y;
    output b.z;
    
    return 0;
}
