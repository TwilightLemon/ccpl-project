struct Row
{
    char cols[10];
};

int main()
{
    struct Row a[5];
    char *p;
    p=&a[2].cols[3];
    p[0]='n';
    *(p+1) ='h';
    output a[2].cols[3];
    output a[2].cols[4];
    char **pp;
    pp=&p;
    **pp='q';
    output a[2].cols[3];
}