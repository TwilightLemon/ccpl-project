struct S{
    int id;
    char a[5];
};

int main(){
    struct S std[2];
    int i;
    for (i = 0; i < 2; i=i+1) {
        std[i].id = i + 1;
        std[i].a[0] = 'a' + i;
    }
    for (i = 0; i < 2; i=i+1) {
        output std[i].id;
        output std[i].a[0];
    }
    return 0;
}