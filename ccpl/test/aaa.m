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
    int i,j,k;
    i=0;j=2;k=3;

    char *p;
    p=&a1[i].a[j][k];
    *p = 'q';
    output a1[i].a[2][3];
    output *p;
    output a1[0].a[j][k];
    return 0;
}