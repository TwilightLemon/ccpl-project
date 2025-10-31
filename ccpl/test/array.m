struct A{
    int id;
    char a[5][10];
};
int main(){
    struct A a1;
    int i,j;
    i=2;
    j=3;
    a1.a[i][j]='x';
    output a1.a[2][3];
    return 0;
}P