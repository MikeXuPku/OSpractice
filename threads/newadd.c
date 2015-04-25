#include<newadd.h>
int newaddfunction(){
	int b = 5;
	#define b 2
	#define f(x) b*(x)
	int y=3;
	printf("%d\n",f(y+1));
	#undef b
	printf("%d\n",f(y+1));
	#define b 3
	printf("%d\n",f(y+1));
	return 0;
}
