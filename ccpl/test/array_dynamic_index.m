# 测试动态数组索引 arr[i]
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
    
    # 使用 arr[i] 动态访问
    sum = 0;
    i = 0;
    while (i < 5) {
        sum = sum + arr[i];
        i = i + 1;
    }
    
    output sum;  # 应该输出150
    
    return 0;
}
