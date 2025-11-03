main()
{
    int i,j;
    int arr1[10],arr2[10][20],arr3[10][20][30];    

    input i;

    arr1[6] = i;
    arr2[6][6] = arr1[6] + 6;
    arr3[6][6][6] = arr2[6][6] + 6;

    if(0) { output "\n"; }

    j = arr3[6][6][6];
    output j;
    output "\n";
}
