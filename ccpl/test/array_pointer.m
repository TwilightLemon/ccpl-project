# 测试数组作为指针使用
int main() {
    int arr[3];
    int *p;
    
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
    
    p = arr;  # 数组名是指向第一个元素的指针
    
    output *p;    # 应该输出1
    
    # 指针算术
    p = p + 4;    # 移到下一个元素(4字节)
    output *p;    # 应该输出2
    
    return 0;
}
