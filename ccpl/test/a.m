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
    int index;
    index=0;
    a1[index].id=1;
    output a1[index].id;
    return 0;
}