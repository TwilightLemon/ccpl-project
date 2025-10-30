int main(){
    char a[5][10];
    char *p;
    p=a + 2*10 + 3;
    *p='t';
    p=p+1;
    *p='w';
    output a[2][3];
    output a[2][4];
    return 0;
}