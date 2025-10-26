struct Point {
    int x;
    int y;
};

struct Rectangle {
    int width;
    int height;
};

struct Point p1;
struct Rectangle rect;

int main() {
    p1.x = 10;
    p1.y = 20;
    
    rect.width = 100;
    rect.height = 50;
    
    output p1.x;
    output p1.y;
    output rect.width;
    output rect.height;
    
    return 0;
}
