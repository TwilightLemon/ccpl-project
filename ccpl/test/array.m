int main(){
    char a[5][10];
    char *p;
    p=a + (2*10 + 3)*4;
    *p='c';
    output a[2][3];
    return 0;
}