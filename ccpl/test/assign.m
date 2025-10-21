char func(int a,char b){
	output a;
	output b;
	output "\n";
	if(a>0){
		return b;
	}
	return '?';
}

void func2(int b){

}

int main()
{
	int a,b,c,d;
	input a;
	b=a+1;
	c=b+2;
	d=-c;
	output d; 
	output "\n";
	char r;
	r = func(a, 'x');
	output r;
	return 0;
}