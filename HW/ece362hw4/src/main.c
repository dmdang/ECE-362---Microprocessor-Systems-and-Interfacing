/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/


#include "stm32f0xx.h"
#include "stm32f0_discovery.h"
			
#include <stdio.h>
#include <stdlib.h>

// prototypes
void serial_init(void);
/* Q1 */ void hello(void);
/* Q2 */ int add2(int a, int b);
/* Q3 */ int add3(int a, int b, int c);
/* Q4 */ int rotate6(int a, int b, int c, int d, int e, int f);
/* Q5 */ int low_pattern(int nth);
/* Q6 */ void get_name(void);
/* Q7 */ int random_sum(void);
/* Q8 */ int fibn(int n);
/* Q9 */ int fun(int a, int b);
/* Q10 */ int sick(int start, int end, int add, int mul, int step);

void check(int q, int t, int e, int a) {
    if (e == a){
        printf("Success for Q%d: Test %d.  Expected %d.  Saw %d.\n",q,t,e,a);
        return;
    }
    printf("Failure for Q%d: Test %d.  Expected %d.  Saw %d.\n",q,t,e,a);
    for(;;);
}

int check_random_sum(void) {
    int arr[20];
    int x;
    int sum = 0;
    arr[0] = random();
    for(x=1; x < sizeof arr / sizeof arr[0]; x += 1)
            arr[x] = arr[x-1] + random();
    for(x=0; x < sizeof arr / sizeof arr[0]; x += 1)
            sum += arr[x];
    return sum;
}

int main() {
    serial_init();

    int x;

    hello();

    for(x=0; x<5; x++) {
        // Should print:
        // 10 + 10 = 20
        // 13 + 9 = 22
        // 16 + 8 = 24
        // 19 + 7 = 26
        // 22 + 6 = 28
        //
        add2(3*x+10, 10-x);
    }

    for(x=0; x<5; x++) {
        // Should print:
        // 0 + 20 + 0 = 20
        // 3 + 19 + 31 = 53
        // 6 + 18 + 62 = 86
        // 9 + 17 + 29 = 55
        // 12 + 16 + 60 = 88
        add3(3*x, 20-x, (31*x)&0x3f);
    }

    for(x=0; x<5; x++) {
        const static int results[] = { -5,-5,-14,-7,-8 };
        const static int in[][6] = {
            { 0,1,2,3,4,5 },
            { 1,2,3,4,5,0 },
            { 4,3,2,1,0,6 },
            { 8,0,9,1,2,3 },
            { 2,4,0,0,0,2 },
        };
        int res =
        rotate6(in[x][0],in[x][1],in[x][2],in[x][3],in[x][4],in[x][5]);
        check(4,x,results[x],res);
    }

    for(x=0; x<5; x++) {
        const static int in[] = { 10,30,50,70,90 };
        const static int results[] = { 49,89,156,196,236 };
        int res = low_pattern(in[x]);
        check(5,x,results[x],res);
    }

    get_name();

    for(x=0; x<5; x++) {
        srand(x);
        const static int results[] = { -251258086,132880400,-1630464767,-1246326290,-862187768 };
        //int res = check_random_sum();
        int res = random_sum();
        check(7,x,results[x],res);
    }

    for(x=0; x<5; x++) {
        const static int in[] = { 1,4,16,32,64 };
        const static int results[] = { 1,3,987,2178309,1640636603 };
        int res = fibn(in[x]);
       check(8,x,results[x],res);
    }

    for(x=0; x<5; x++) {
        const static int in[][2] = {
            { 1, 2 },
            { 3, 4 },
            { 10, 20 },
            { 15, 55 },
            { 25, 98 },
        };
        const static int results[] = { 779,2091,90200,1470875,7456055 };
        int res = fun(in[x][0], in[x][1]);
        check(9,x,results[x],res);
    }

    for(x=0; x<5; x++) {
        const static int in[][5] = {
            { 1, 2, 1, 2, 1 },
            { 3, 4, 1, 2, 1 },
            { 3,10, 2, 5, 2 },
            { 6,85, 3, 7, 3 },
            { 5,90, 4, 9, 5 },
        };
        const static int results[] = { 14,46,710,272727,271890 };
        int res = sick(in[x][0],in[x][1],in[x][2],in[x][3],in[x][4]);
        check(10,x,results[x],res);
    }

    printf("Good job.\n");
    for(;;);
    return 0;
}
