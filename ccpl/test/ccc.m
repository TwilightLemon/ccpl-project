struct A{
    int id,q;
};

int main(){
    struct A a1;
    int a[5];
    a[1]=9;
    a1.q=a[1];
    output a1.q;
    return 0;
}