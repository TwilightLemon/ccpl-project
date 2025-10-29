main()
{
	int i;
	int maxv;
	input i;
	while(i<10)
	{ 
		output i; 
		i=i+1;
	}
	maxv = max(5,8);
	output maxv;
	output "\n";
}

max(a,b){
	if(a>b){
		return a;
	}
	return b;
}