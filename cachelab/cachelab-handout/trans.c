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
// cache :5,1,5 5个组，每组一行，block=32字节
char transpose_submit_desc[] = "Transpose submission";
void solve_32(int M, int N, int A[N][M], int B[M][N]);
void solve_64(int M, int N, int A[N][M], int B[M][N]);
void solve_61(int M, int N, int A[N][M], int B[M][N]);
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    switch (M) {
        case 32:
            solve_32(M, N, A, B);
            break;
        case 64:
            solve_64(M, N, A, B);
            break;
        case 61:
            solve_61(M, N, A, B);
            break;
    }
}

void solve_32(int M, int N, int A[N][M], int B[M][N])
{
    int i,j,k,l;
    int tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7,tmp8;
    for(i=0;i<M;i+=8){
            for(j=0;j<N;j+=8){
                for(k=i;k<i+8;k++){
                    if(i!=j){
                        for(l=j;l<j+8;l++){
                            B[l][k]=A[k][l];
                        }
                    }else{
                        tmp1=A[k][j];
                        tmp2=A[k][j+1];
                        tmp3=A[k][j+2];
                        tmp4=A[k][j+3];
                        tmp5=A[k][j+4];
                        tmp6=A[k][j+5];
                        tmp7=A[k][j+6];
                        tmp8=A[k][j+7];
                        B[j][k]=tmp1;
                        B[j+1][k]=tmp2;
                        B[j+2][k]=tmp3;
                        B[j+3][k]=tmp4;
                        B[j+4][k]=tmp5;
                        B[j+5][k]=tmp6;
                        B[j+6][k]=tmp7;
                        B[j+7][k]=tmp8;
                    }
                }
            }
        }
}
// 这是4*4的优化版本，具体思想是，尽量多地利用整个cache
void solve_64(int M, int N, int A[N][M], int B[M][N])
{
    int i, j,k;
    int tmp1,tmp2,tmp3,tmp4,tmp5,tmp6,tmp7,tmp8;
    for(i=0;i<M;i+=4){
            for(j=0;j<N;j+=4){
                for(k=i;k<i+4;k+=2){
                        tmp1=A[k][j];
                        tmp2=A[k][j+1];
                        tmp3=A[k][j+2];
                        tmp4=A[k][j+3];
                        tmp5=A[k+1][j];
                        tmp6=A[k+1][j+1];
                        tmp7=A[k+1][j+2];
                        tmp8=A[k+1][j+3];
                        B[j][k]=tmp1;
                        B[j+1][k]=tmp2;
                        B[j+2][k]=tmp3;
                        B[j+3][k]=tmp4;
                        B[j][k+1]=tmp5;
                        B[j+1][k+1]=tmp6;
                        B[j+2][k+1]=tmp7;
                        B[j+3][k+1]=tmp8;
                }
            }
        }
}


void solve_61(int M, int N, int A[N][M], int B[M][N])
{
    int i,j,k,l;
    for(i=0;i<N;i+=16){
            for(j=0;j<M;j+=16){
                for(k=i;k<N&&k<16+i;k++){
                    for(l=j;l<16+j&&l<M;l++){
                        B[l][k]=A[k][l];
                    }
                }
            }
        }
}

void partB(int M, int N, int A[N][M], int B[M][N]) {}
void transpose_submit3(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i += 8) {      // 枚举每八行
        for (j = 0; j < M; j += 4) {  // 枚举每八列
            int temp1, temp2, temp3, temp4, t5, t6, t7, t8;
            for (int k = i; k < i + 8; ++k) {  // 枚举0~8中的每一行，一行八列

                if (j != 0 && k == i + 3) {
                    B[j][k] = t5;
                    B[j + 1][k] = t6;
                    B[j + 2][k] = t7;
                    B[j + 3][k] = t8;
                } else {
                    temp1 = A[k][j];
                    temp2 = A[k][j + 1];
                    temp3 = A[k][j + 2];
                    temp4 = A[k][j + 3];
                    B[j][k] = temp1;
                    B[j + 1][k] = temp2;
                    B[j + 2][k] = temp3;
                    B[j + 3][k] = temp4;
                }
                if (k == i + 3) {
                    t5 = A[k][j + 4];
                    t6 = A[k][j + 5];
                    t7 = A[k][j + 6];
                    t8 = A[k][j + 7];
                }
            }
        }
    }
}

void transpose_submit2(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i += 8) {               // 枚举每八行
        for (j = 0; j < M; j += 8) {           // 枚举每八列
            for (int k = i; k < i + 8; ++k) {  // 枚举0~8中的每一行，一行八列
                int temp1 = A[k][j];
                int temp2 = A[k][j + 1];
                int temp3 = A[k][j + 2];
                int temp4 = A[k][j + 3];
                int temp5 = A[k][j + 4];
                int temp6 = A[k][j + 5];
                int temp7 = A[k][j + 6];
                int temp8 = A[k][j + 7];
                if (k & 1) {
                    B[j][k] = temp1;
                    B[j + 1][k] = temp2;
                    B[j + 2][k] = temp3;
                    B[j + 3][k] = temp4;
                    B[j + 4][k] = temp5;
                    B[j + 5][k] = temp6;
                    B[j + 6][k] = temp7;
                    B[j + 7][k] = temp8;
                } else {
                    B[j + 4][k] = temp5;
                    B[j + 5][k] = temp6;
                    B[j + 6][k] = temp7;
                    B[j + 7][k] = temp8;
                    B[j][k] = temp1;
                    B[j + 1][k] = temp2;
                    B[j + 2][k] = temp3;
                    B[j + 3][k] = temp4;
                }
            }
        }
    }
}

/*
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started.
 */

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
    // registerTransFunction(transpose_submit3, trans_desc);
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
