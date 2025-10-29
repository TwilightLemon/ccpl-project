# 测试动态数组访问
int main() {
    int arr[5];
    int i;
    int sum;
    
    # 初始化数组
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    arr[3] = 40;
    arr[4] = 50;
    
    # 使用动态索引访问数组
    sum = 0;
    i = 0;
    int *ac;
    while (i < 5) {
        ac=arr + i * 4;
        sum = sum + *ac;
        i = i + 1;
    }
    
    output sum;  # 应该输出150
    
    return 0;
}
