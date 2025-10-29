# 测试指针基本操作
int main() {
    int x;
    int *p;
    
    x = 42;
    p = &x;
    
    output *p;  # 应该输出42
    
    *p = 100;
    output x;   # 应该输出100
    
    return 0;
}
