/**
 * 被用来测试stack每次从一个固定的地点开始
*/

int func(int *var){
    printf("the address always start from 0x%x \n", (unsigned int)&var);
}

int main(void){
    int x=10;
    func(&x);
    return 0;
}