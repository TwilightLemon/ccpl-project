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
    p = p + 1;    # 移到下一个元素
    output *p;    # 应该输出2

    int ind;
    ind=2;
    output arr[ind]; #动态计算偏移

    return 0;
}
