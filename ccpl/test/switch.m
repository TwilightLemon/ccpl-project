int main(){
    int x;
    x = 2;
    switch (x) {
        case 1:
        case 2:
            x = x + 10;
            break;
        default:
            x = x + 20;
    }
    output x;
    return 0;
}