// main.c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define RUNS 30
#define EPS 1e-9

// ---------- Function prototypes ----------
void daxpy_asm(double* z, const double* x, const double* y, const double* pA, size_t n);

// C DAXPY kernel (used as correctness reference)
void daxpy_c(double* z, const double* x, const double* y, double A, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        z[i] = A * x[i] + y[i];
}

// ---------- Helper functions ----------
static void init_data(double* x, double* y, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        x[i] = (double)(i % 1000) * 0.001;
        y[i] = (double)((i * 7) % 1000) * 0.002;
    }
}

static int check_correctness(const double* ref, const double* test, size_t n)
{
    for (size_t i = 0; i < n; ++i) {
        double diff = fabs(ref[i] - test[i]);
        if (diff > EPS) {
            printf("Mismatch at index %zu: ref=%.15f test=%.15f diff=%.3e\n",
                i, ref[i], test[i], diff);
            return 0;
        }
    }
    return 1;
}

static void print_first_ten(const char* label, const double* z, size_t n)
{
    printf("%s (first 10):\n", label);
    for (size_t i = 0; i < 10 && i < n; ++i)
        printf("  Z[%2zu] = %.6f\n", i, z[i]);
}

static double time_c(double* z, const double* x, const double* y, double A, size_t n)
{
    clock_t start = clock();
    daxpy_c(z, x, y, A, n);
    return (double)(clock() - start) / CLOCKS_PER_SEC;
}

static double time_asm(double* z, const double* x, const double* y, double A, size_t n)
{
    clock_t start = clock();
    daxpy_asm(z, x, y, &A, n);   // A passed by pointer
    return (double)(clock() - start) / CLOCKS_PER_SEC;
}

// ---------- Verification with example from specs ----------
static void verify_with_example(void)
{
    printf("\n=========================================\n");
    printf(" PROCESS VERIFICATION\n");
    printf("=========================================\n");
    
    double A = 2.0;
    size_t n = 3;
    double x[] = {1.0, 2.0, 3.0};
    double y[] = {11.0, 12.0, 13.0};
    double zC[3] = {0};
    double zA[3] = {0};
    
    printf("Input:\n");
    printf("  A --> %.1f\n", A);
    printf("  x --> %.1f, %.1f, %.1f\n", x[0], x[1], x[2]);
    printf("  y --> %.1f, %.1f, %.1f\n", y[0], y[1], y[2]);
    
    printf("\nProcess (DAXPY: Z = A*X + Y):\n");
    for (size_t i = 0; i < n; i++) {
        double expected = A * x[i] + y[i];
        printf("  z[%zu] = %.1f * %.1f + %.1f = %.1f\n", 
               i, A, x[i], y[i], expected);
    }
    
    // Compute with both kernels
    daxpy_c(zC, x, y, A, n);
    daxpy_asm(zA, x, y, &A, n);
    
    printf("\nOutput (C kernel):\n");
    printf("  z --> %.1f, %.1f, %.1f\n", zC[0], zC[1], zC[2]);
    
    printf("\nOutput (x86-64 ASM kernel):\n");
    printf("  z --> %.1f, %.1f, %.1f\n", zA[0], zA[1], zA[2]);
    
    int ok = check_correctness(zC, zA, n);
    printf("\nCorrectness: %s\n", ok ? "PASSED - ASM output matches C output" : "FAILED");
}

// ---------- Runner ----------
static void run_test(size_t n)
{
    printf("\n=========================================\n");
    printf(" Vector size n = %zu (2^%d)\n", n, (int)log2((double)n));
    printf("=========================================\n");

    double A = 2.0;

    double* x = malloc(n * sizeof(double));
    double* y = malloc(n * sizeof(double));
    double* zC = malloc(n * sizeof(double));
    double* zA = malloc(n * sizeof(double));

    if (!x || !y || !zC || !zA) {
        printf("Memory allocation failed.\n");
        free(x); free(y); free(zC); free(zA);
        return;
    }

    init_data(x, y, n);

    double sumC = 0, sumA = 0;
    for (int i = 0; i < RUNS; i++) sumC += time_c(zC, x, y, A, n);
    for (int i = 0; i < RUNS; i++) sumA += time_asm(zA, x, y, A, n);

    double avgC = sumC / RUNS;
    double avgA = sumA / RUNS;

    int ok = check_correctness(zC, zA, n);
    printf("Correctness check: %s\n", ok ? "PASSED - x86-64 kernel output is correct" : "FAILED");

    print_first_ten("C version result", zC, n);
    print_first_ten("x86-64 ASM version result", zA, n);

    printf("\nAverage kernel execution time (%d runs):\n", RUNS);
    printf("  C kernel       : %.6f sec\n", avgC);
    printf("  x86-64 kernel  : %.6f sec\n", avgA);

    free(x);
    free(y);
    free(zC);
    free(zA);
}

// ---------- main ----------
int main(void)
{
    // Verify with example from specifications
    verify_with_example();
    
    // Performance testing with specified vector sizes
    size_t n1 = (size_t)1 << 20;  // 2^20
    size_t n2 = (size_t)1 << 24;  // 2^24
    size_t n3 = (size_t)1 << 28;  // 2^28

    run_test(n1);
    run_test(n2);
    run_test(n3);

    printf("\n=========================================\n");
    printf(" All tests completed!\n");
    printf("=========================================\n");

    return 0;
}

