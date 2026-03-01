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

void trans_block(int M, int N, int A[N][M], int B[M][N], int x_start, int y_start);
void trans_block8(int M, int N, int A[N][M], int B[M][N], int x_start, int y_start);
void trans_block4(int M, int N, int A[N][M], int B[M][N], int x_start, int y_start);
void trans_block_remained(int M, int N, int A[N][M], int B[M][N], int x_start, int y_start);
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
    int block_size;
    if (M == 32 && N == 32) {
        block_size = 8;
        for (int i = 0; i < N; i+=block_size){
            for(int j = 0; j < M; j+=block_size){
                trans_block8(M, N, A, B, i, j);
            }
        }
    }else if(M == 64 && N == 64) {
        block_size = 8;
        for (int i = 0; i < M; i+=block_size){
            for(int j = 0; j < N; j+=block_size){
                trans_block4(M, N, A, B, i, j);
            }
        }
    }else{
        block_size = 16;
        for(int i = 0;i < N;i+=block_size){
            for(int j = 0; j< M; j+=block_size){
                for(int r = i; r<i+block_size && r<N; r++){
                    for(int c = j; c<j+block_size && c<M;c++){
                        B[c][r] = A[r][c];
                    }
                }
            }
        }
        // block_size = 8;
        // int i, j;
        // for (i = 0; i <= N-block_size; i+=block_size){
        //     for(j = 0; j <= M-block_size; j+=block_size){
        //         trans_block8(M, N, A, B, i, j);
        //     }
        // }
        // int ii;
        // int jj;
        // for(ii = i; ii < N; ii++){
        //     for(jj = 0; jj <= M - block_size; jj += block_size){
        //         trans_block(M, N, A, B, ii, jj);
        //     }
        // }
        // if(j <= M-block_size){
        //     for(ii = 0; ii <= N - block_size; ii += block_size){
        //         trans_block(M, N, A, B, ii, j);
        //     }
        // }
        // trans_block_remained(M, N, A, B, ii, jj);
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
void trans_block(int M, int N, int A[N][M], int B[M][N], int x_start, int y_start){
    int v0, v1, v2, v3, v4, v5, v6, v7;
    v0 = A[x_start][y_start];
    v1 = A[x_start][y_start+1];
    v2 = A[x_start][y_start+2];
    v3 = A[x_start][y_start+3];
    v4 = A[x_start][y_start+4];
    v5 = A[x_start][y_start+5];
    v6 = A[x_start][y_start+6];
    v7 = A[x_start][y_start+7];
    B[y_start][x_start] = v0;
    B[y_start+1][x_start] = v1;
    B[y_start+2][x_start] = v2;
    B[y_start+3][x_start] = v3;
    B[y_start+4][x_start] = v4;
    B[y_start+5][x_start] = v5;
    B[y_start+6][x_start] = v6;
    B[y_start+7][x_start] = v7;
}
void trans_block_remained(int M, int N, int A[N][M], int B[M][N], int x_start, int y_start){
    for(int i = x_start; i < N; i++){
        for (int j = 0; j < y_start; j++){
            int tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
    for(int i = 0;i < x_start; i++){
        for (int j = y_start; j < M; j++){
            int tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
    for(int i = x_start; i < N; i++){
        for (int j = y_start; j < M; j++){
            int tmp = A[i][j];
            B[j][i] = tmp;
        }
    }
}
void trans_block8(int M, int N, int A[N][M], int B[M][N], int x_start, int y_start){
    int i, v0, v1, v2, v3, v4, v5, v6, v7;
    for(i = x_start; i<x_start+8; i++){
        v0 = A[i][y_start];
        v1 = A[i][y_start+1];
        v2 = A[i][y_start+2];
        v3 = A[i][y_start+3];
        v4 = A[i][y_start+4];
        v5 = A[i][y_start+5];
        v6 = A[i][y_start+6];
        v7 = A[i][y_start+7];
        B[y_start][i] = v0;
        B[y_start+1][i] = v1;
        B[y_start+2][i] = v2;
        B[y_start+3][i] = v3;
        B[y_start+4][i] = v4;
        B[y_start+5][i] = v5;
        B[y_start+6][i] = v6;
        B[y_start+7][i] = v7;
    }
}
void trans_block4(int M, int N, int A[N][M], int B[M][N], int x_start, int y_start){
    int i, v0, v1, v2, v3, v4, v5, v6, v7;
    for(i = 0; i < 4; i++){
        v0 = A[x_start+i][y_start];
        v1 = A[x_start+i][y_start+1];
        v2 = A[x_start+i][y_start+2];
        v3 = A[x_start+i][y_start+3];
        v4 = A[x_start+i][y_start+4];
        v5 = A[x_start+i][y_start+5];
        v6 = A[x_start+i][y_start+6];
        v7 = A[x_start+i][y_start+7];
        B[y_start][x_start+i] = v0;
        B[y_start+1][x_start+i] = v1;
        B[y_start+2][x_start+i] = v2;
        B[y_start+3][x_start+i] = v3;
        B[y_start][x_start+i+4] = v4;
        B[y_start+1][x_start+i+4] = v5;
        B[y_start+2][x_start+i+4] = v6;
        B[y_start+3][x_start+i+4] = v7;
    }
    for(i = 0; i<4; i++){
        v0 = B[y_start+i][x_start+4];
        v1 = B[y_start+i][x_start+5];
        v2 = B[y_start+i][x_start+6];
        v3 = B[y_start+i][x_start+7];
        v4 = A[x_start+4][y_start+i];
        v5 = A[x_start+5][y_start+i];
        v6 = A[x_start+6][y_start+i];
        v7 = A[x_start+7][y_start+i];
        B[y_start+i][x_start+4] = v4;
        B[y_start+i][x_start+5] = v5;
        B[y_start+i][x_start+6] = v6;
        B[y_start+i][x_start+7] = v7;
        B[y_start+i+4][x_start] = v0;
        B[y_start+i+4][x_start+1] = v1;
        B[y_start+i+4][x_start+2] = v2;
        B[y_start+i+4][x_start+3] = v3;
    }
    for (int i = 4; i < 8; i++){
        v0 = A[x_start+i][y_start+4];
        v1 = A[x_start+i][y_start+5];
        v2 = A[x_start+i][y_start+6];
        v3 = A[x_start+i][y_start+7];
        B[y_start+4][x_start+i] = v0;
        B[y_start+5][x_start+i] = v1;
        B[y_start+6][x_start+i] = v2;
        B[y_start+7][x_start+i] = v3;
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
    //registerTransFunction(trans, trans_desc); 

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

