struct Point {
    int x;
    int y;
    char label[3];
};

struct Line {
    struct Point start;
    struct Point end;
};

struct Line line;
struct Point p;

int main() {
    p.x = 10;
    p.y = 20;
    line.start.x = 0;
    line.end.x = 100;
    output p.x;
    output line.start.x;
    return 0;
}
