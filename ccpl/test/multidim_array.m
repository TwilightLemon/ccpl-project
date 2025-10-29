# 测试多维数组动态访问
int main() {
    int matrix[2][3];
    int i;
    int j;
    int val;
    
    # 初始化2x3矩阵
    matrix[0][0] = 1;
    matrix[0][1] = 2;
    matrix[0][2] = 3;
    matrix[1][0] = 4;
    matrix[1][1] = 5;
    matrix[1][2] = 6;
    
    # 使用动态索引
    i = 1;
    j = 2;
    val = matrix[i][j];
    
    output val;  # 应该输出6
    
    return 0;
}
