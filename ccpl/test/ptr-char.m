main()
{
    char a,*pa,b,c,d;
    char *ptr;
    input d;
    c = 'c';
    b = 'b';
    input a;

    if(0) { output "\n"; }

    output a;
    output b;
    output c;
    output d;
    output "\n";

    pa = &a;
    *pa = 'A';
    output a;

    ptr = pa;
    *ptr = 'B';
    output a;
    output "\n";
}
