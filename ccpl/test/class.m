main(){
    char a;
    input a;

    char *p;
    p = &a;
    output p;   # addr of a
    output "\n";
    output *p;  # a

    int force;
    force=*p;
    output "\n";
    output force;   # ascii value of a
}