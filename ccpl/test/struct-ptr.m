struct student
{
    int *ptr;
    char name[10];
    int score[10];
};

struct group
{
    int num;
    char name[10];
    struct student stu[10];
};

struct class
{
    int num;
    char name[10];
    struct group grp[10];
};

main()
{
    int i,j,*pi;
    char a,b,*pc;
    struct class c1;
   
    c1.grp[2].stu[3].ptr = &c1.num;
    pi = c1.grp[2].stu[3].ptr;
    *pi = 999;

    i = c1.num;

    output i;
    output "\n";
}
