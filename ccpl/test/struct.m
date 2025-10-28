struct student
{
    int num;
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

int main()
{
    int i,j;    
    char a,b;
    struct class c1;
    c1.num = 1;
    c1.grp[2].num = 2;
    c1.grp[2].stu[3].name[1] = 'b';
    c1.grp[2].stu[3].name[0] = 'a';

    if(0) { output "\n"; }

    i = c1.num;
    j = c1.grp[2].num;
    a = c1.grp[2].stu[3].name[0];
    b = c1.grp[2].stu[3].name[1];

    if(0) { output "\n"; }

    output i;
    output j;
    output a;
    output b;
    output "\n";
}
