struct A{
int id;
char name_list[5][10];
};

int main(){
    struct A a1;
    int x,y;
    x=2;
    y=3;
    a1.id=9;
    char *k;
    k=a1.name_list + x * 40 + y * 8;
    *k='x';
    output a1.name_list[2][3]; # 应该输出x
    return 0;
}