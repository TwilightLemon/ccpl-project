struct A{
int id;
char name_list[5][10];
};

int main(){
    struct A a1;
    a1.id=9;
    a1.name_list[2][3]='x';
    return 0;
}