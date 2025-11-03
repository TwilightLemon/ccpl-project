struct student
{
    int num;
    int phone;
    int score;
};

main()
{
    int i,j,k;    
    struct student zs;
    
    input i;
    input j;
    input k;

    zs.num = i;
    zs.phone = j;
    zs.score = k;

    if(0) { output "\n"; }

    i = zs.score + 100;
    j = zs.phone + 200;
    k = zs.num + 300;

    if(0) { output "\n"; }

    output i;
    output j;
    output k;
    output "\n";
}
