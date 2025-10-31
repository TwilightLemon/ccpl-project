struct Row
{
    char cols[10];
};

int main()
{
    struct Row a[5];
    char *p;
    p=&a[2].cols[3];
    *p='n'; #p[0]='n';
    p=p+1; #p[1] ='h'; -> *(p+1)='h';
    *p ='h';
    output a[2].cols[3];
    output a[2].cols[4];
}