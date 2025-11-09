struct BB{
    char q[2];
};
struct A{
    int id;
    char a[4][5];
    struct BB b[2];
};
int main(){
    struct A a1[2];
    int i=0,j=2,k=3;

    a1[0].a[2][3]='z';
    output a1[0].a[2][3];

    char *p=&a1[i].a[j][k];
    *p = 'q';
    output a1[i].a[2][3];
    *p='R'; #重写p指向内存
    output *p;
    *p = 'w'; #重写p指向内存
    output a1[i].a[j][k];
    return 0;
}