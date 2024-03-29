/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";

void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii;

    int b = 4; 
    for (i = 0; i < N; i += b) {
        for (j = 0; j < M; j += b) {
            for(ii = i; ii < i + b; ii++) {
                int tmp0 = A[ii][j];
                int tmp1 = A[ii][ j + 1];
                int tmp2 = A[ii][ j + 2];
                int tmp3 = A[ii][ j + 3];
                          

                B[j][ii] = tmp0;
                B[j + 1][ii] = tmp1;
                B[j + 2][ii] = tmp2;
                B[j + 3][ii] = tmp3;
              
            }
        }
    }    
}

void transpose_submit2(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, ii;

    int b = 8; 
    for (i = 0; i < N; i += b) {
        for (j = 0; j < M; j += b) {
            for(ii = i; ii < i + b; ii++) {
                int tmp0 = A[ii][j];
                int tmp1 = A[ii][ j + 1];
                int tmp2 = A[ii][ j + 2];
                int tmp3 = A[ii][ j + 3];
                int tmp4 = A[ii][ j + 4];
                int tmp5 = A[ii][ j + 5];
                int tmp6 = A[ii][ j + 6];
                int tmp7 = A[ii][ j + 7];

                B[j][ii] = tmp0;
                B[j + 1][ii] = tmp1;
                B[j + 2][ii] = tmp2;
                B[j + 3][ii] = tmp3;
                B[j + 4][ii] = tmp4;
                B[j + 5][ii] = tmp5;
                B[j + 6][ii] = tmp6;
                B[j + 7][ii] = tmp7;
            }
        }
    }    
}

/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

void transpose_submit1(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp, ii,  jj;

    int b = 8; 
    for (i = 0; i < N; i += b) {
        for (j = 0; j < M; j += b) {
            for(ii = i; ii < i + b; ii++) {
                for (jj = j; jj < j + b; jj ++) {
                    tmp = A[ii][jj]; 
                    B[jj][ii] = tmp;
                }
            }

        }
    }    
}

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

