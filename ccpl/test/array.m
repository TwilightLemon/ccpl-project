struct A{
    int id;
    char a[5][10];
};
int main(){
    struct A a1;
    int i,j;
    i=2;
    j=3;
    char c;
    c=a1.a[i][j]='x';
    output a1.a[i][j];
    output a1.a[2][3];
    output c;
    return 0;
}