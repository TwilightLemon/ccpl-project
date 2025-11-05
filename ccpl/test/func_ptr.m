void func(int *a){
    *a=3;
}
main(){
    int a;
    a=1;
    output a;
    func(&a);
    output a;
    return 0;
}
