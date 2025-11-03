main()
{
    int i,j;
    int arr1[10];    

    input i;

    j = 0;
    while(j<10)
    {
        arr1[j] = i;
        i = i + 1;
        j=j+1;
    }

    if(0) { output "\n"; }

    while(j>0)
    {
        j = j - 1;
        i = arr1[j];
        output i;
    }
    
    output "\n";
}
