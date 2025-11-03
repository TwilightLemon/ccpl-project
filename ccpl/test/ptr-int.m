main()
{
	int a,*pa,b,c,d;
    int *ptr;
	input a;
	b=a+10;
	c=b-20;
	d=c*30;
	output a;
	output b;
	output c;
	output d; 
	output "\n";

    pa = &a;
    *pa = 111;
    output a;

    ptr = pa;
    *ptr = 222;
    output a;
    output "\n";    
}
