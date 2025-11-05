void func(int a[5]){ # or int *a
    a[0]=3; #or *(a+0) = 3;
}
main(){
    int a[5];
    a[0]=1;
    output a[0];
    func(a);# or &a[0]
    output a[0];
    return 0;
}
