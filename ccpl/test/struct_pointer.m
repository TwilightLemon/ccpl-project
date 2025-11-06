struct A{
    int a,b,c;
};

main(){
    struct A a1,*p=&a1;
    p->c=10;
    output p->c;
    return 0;
}