struct Info{
    int id;
    char name;
};
struct Color {
    struct Info info;
    char r;
    char g;
    char b;
};
struct Point {
    int x;
    int y;
};

struct Rectangle {
    int width;
    int height;
    struct Color color;
    struct Point pos;
};

struct Rectangle rect;
struct Info info;
struct Color color;

int main() {
    rect.width = 100;
    rect.height = 50;

    rect.pos.x = 10;
    rect.pos.y = 20;
    
    output rect.width;
    output rect.height;
    output rect.pos.x;
    output rect.pos.y;

    return 0;
}
