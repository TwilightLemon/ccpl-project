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
    struct Vector v1;
    struct Vector v2;
    
    v1.x = 1;
    v1.y = 2;
    v1.z = 3;
    
    v2.x = 4;
    v2.y = 5;
    v2.z = 6;
    
    output v1.x;
    output v1.y;
    output v1.z;
    
    output v2.x;
    output v2.y;
    output v2.z;
    
    return 0;
}
