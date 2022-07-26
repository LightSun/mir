
#include <stdio.h>
int add(int a, int b){
   int sum = 0;	
   for(int i = 1 ; i < 11 ; i ++){
	  sum += i;
   } 
   printf("a=%d\n",a);	
   return sum + a + b;
}
