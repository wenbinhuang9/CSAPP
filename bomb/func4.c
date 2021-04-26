#include <stdio.h>

int func4(int edi, int esi, int edx) {
    int ecx = (edx - esi) >> 31;
    int eax = (ecx + (edx - esi)) >> 1;
    ecx = eax + esi;

    if(ecx <= edi ) {
        if (ecx >= edi) {
            return 0;
        }
        eax =  func4(edi, ecx + 1, edx);

        return 2 * eax + 1;
    }else {
        //ecx > edi
        edx = ecx - 1; 
        eax = func4(edi, esi, edx);

        return 2 * eax; 
    }

}
int fun(int a1, int a2, int x){
    int b = (a1 - a2) >> 31;
    int result = ((a1-a2) + b) >> 1;
    b = result + a2;
    if(b == x) return 0;
    if(b < x) {
        result = fun(a1, b + 1, x);
        return result * 2 + 1;
    }else{
        result = fun(b - 1, a2, x);
        return result * 2;
    }
}
int main(void){
    for(int i = 0; i <= 0xe; i++){
        if(func4(i, 0, 0xe) == 0){
            printf("%d\n",i) ;
            return 0;
        }
    }
    return 0; 
}