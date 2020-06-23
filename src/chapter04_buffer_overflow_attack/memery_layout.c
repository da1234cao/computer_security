/**
 * 使用gdb调试该程序，展示内存布局
*/
#include <stdio.h>

void func(int a,int b){
	int x,y;
	x = a + b;
	y = a - b;
}

int main(void){
	int x=0;
	int y=0;
	func(4,3);
	return 0;
}
