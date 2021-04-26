
#include <stdio.h>

int sum (int a, int b) {
    return a + b;
}

int cmp(int a, int b) {
    if (a > b) {
        return 1;
    }else {
        return 0; 
    }
}
int sum_arr(int* arr, int n ) {
    int s = 0;
    for(int i = 0; i < n; i++) {
        s += arr[i];
    }
    return s;
}
int main() {
    int c = sum(10, 20);
    printf("%d\n", c);
 
    int cmpvalue = cmp(11, 100);

    printf("%d\n", cmpvalue);

    int arr[] = {1,2,3,4,5};

    int s = sum_arr(arr, 5);
    printf("sum value is %d\n", s);
    return 0;
}