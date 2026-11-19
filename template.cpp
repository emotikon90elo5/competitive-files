#include <bits/stdc++.h> 
#include <cstdio>

using namespace std;
typedef long long ll

inline void putUI(unsigned int n) {
     if(n==0)
     {
     	putc_unlocked(48,stdout); return; 
     }
	 char tab[12];
     int p = 0;
     while(n != 0) {
        tab[p++] = (n % 10) + 48;
         n /= 10;
     }
     while(p--) putc_unlocked(tab[p], stdout);
}
inline void putDB(long double n)
{
	const int eps = 3;
	long long k = (long long )(n*1000);
	 char arr[16],i;
     int p = 0;
    for(i=0;i<eps;i++) {
        arr[p++] = (k % 10) + 48;
        k /= 10;
    }
    arr[p++]='.';
    do{
    	arr[p++]=(k%10)+48;
    	k/=10;
    }while(k!=0); 
     while(p--) 
         putc_unlocked(arr[p], stdout);
}
inline void readINT(int *n) //ujemne i dodatnie
{
    register char c = 0, 
	znak_liczby=1;
    while (c < 33) c=getc_unlocked(stdin);
    if(c==45) {znak_liczby=-1;  c=getc_unlocked(stdin);}
    (*n) = 0;
	while (c>32) {(*n)=(*n)*10 + c-48; c=getc_unlocked(stdin);} 
    (*n)*=znak_liczby;
} 
inline int readS(char *a)//funkcja zwraca liczbę znaków
{
	int dl=0;
    register char c = 0;
    while (c < 33) c=getchar_unlocked(); 
   	dl = 0;
    while (c>32) {
		a[dl++] = c; 
		c=getchar_unlocked();
	}
	a[dl]=0;
	return dl;
}


int main() {
    ios_base::sync_with_stdio(false); cin.tie(0);

    return 0;
}


