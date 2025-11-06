struct Q{
    char c,d;
};
struct A{
    int id,q;
    struct Q r;
};

struct A a1;
int main(){
    int *p=&a1;
    p[3]='c';
    output a1.r.d;
    return 0;
}