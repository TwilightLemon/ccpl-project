main()
{
	int i,j;
	i = 0;
	input j;

	while(i<j)
	{ 
		output i; 
		i=i+1;
		if(i>10)
		{
			output "break";
			break;
		}		
	}
	output "\n";

	while(i<j)
	{ 
		output i; 
		i=i+1;
		if(i==10)
		{
			output "continue";
			continue;
		}	
	}
	output "\n";

}
