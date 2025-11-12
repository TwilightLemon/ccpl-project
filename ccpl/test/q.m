main()
{
    int a,b,c,d,e,sumd,sume,i,j,k;
    input a;
    input b;
    input c;

    sumd = 0;
    sume = 0;
    k = 12;

    while(k > 0)
    {
        j = 15;
        while(j > 0)
        {
            i = 18;
            while(i > 0)
            {
                d = a + b * c - (a + c) / b + 9;
                e = a + b * c - (c - a) / b + 13;
                sumd = sumd + d;
                sume = sume + e + d;
                i = i - 1;
            }
            j = j - 1;
        }
        k = k - 1;
    }

    output sumd;
    output " ";
    output sume;
    output "\n";
}