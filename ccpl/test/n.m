main() 
{
    int i, j, k;
    k = 0;
    while (k < 10) {
        i = 0;
        while (i < 10) {
            j = 2 * i + 9; 
            output j;
            output "\n";
            i = i + 1;
        }
        k = k + 1;
    }
}