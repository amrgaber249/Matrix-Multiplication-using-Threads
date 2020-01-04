#include <stdio.h>
#include <stdlib.h>
#include<pthread.h>
#include <time.h>

#define verbose 2
/*
'0' : no info
'1' : basic info
'2' : adds info about the reading from files
'3' : adds info about threads
*/

//initializing global variables
int **matA, **matB, **matC;
int row, mid, col, nthreads=0;
//take 2 matrices from "input.txt" file
void mat_read()
{
    //clear output file
    FILE *fp;
    fp = fopen("output.txt", "w");
    fclose(fp);

    //take input matrix
    FILE *file;
    file = fopen("input.txt", "r+");

    //read the size of the first matrix
    fscanf(file, "%d", &row);
    fscanf(file, "%d", &mid);

    if(verbose>1)
        printf("\n[1st matrix] (%d, %d)\n", row,mid);

    //create the first matrix
    matA = malloc( sizeof *matA * row );
    if ( matA )
    {
        for ( size_t i = 0; i < row; i++ )
            matA[i] = malloc( sizeof *matA[i] * mid );
    }

    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < mid; j++)
        {
            fscanf(file, "%d", &matA[i][j]);
        }
    }
    if(verbose>1)
    {
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < mid; j++)
            {
                printf("%d ", matA[i][j]);
            }
            printf("\n");
        }
    }

    //read the size of the second matrix
    fscanf(file, "%d", &mid);
    fscanf(file, "%d", &col);

    if(verbose>1)
        printf("\n[2nd matrix] (%d, %d)\n", mid,col);

    //create the second matrix
    matB = malloc( sizeof *matB * mid );
    if ( matB )
    {
        for ( size_t i = 0; i < mid; i++ )
            matB[i] = malloc( sizeof *matB[i] * col );
    }


    for (int i = 0; i < mid; i++)
    {
        for (int j = 0; j < col; j++)
        {
            fscanf(file, "%d", &matB[i][j]);
        }
    }
    if(verbose>1)
    {
        for (int i = 0; i < mid; i++)
        {
            for (int j = 0; j < col; j++)
            {
                printf("%d ", matB[i][j]);
            }
            printf("\n");
        }
    }
    matC = malloc( sizeof *matC * row );
    if ( matC )
    {
        for ( size_t i = 0; i < row; i++ )
            matC[i] = malloc( sizeof *matC[i] * col );
    }
    fclose(file);
}


//apply the dot product for each thread
void* multi(void* arg)
{
    //take the first element in the argument pointer
    int t = (int)arg;

    if(verbose>2)
        printf("n = %d\n",t);
    // get the number of thread
    t = (int)t;
    //find out where to start/end this specific thread
    int start = (t*row)/nthreads;
    int end = ((t+1)*row)/nthreads;

    for (int i = start; i<end; i++)
        for (int j = 0; j < col; j++)
        {
            //initialize the cell so it doesn't add over a garbage number
            matC[i][j] = 0;
            for (int k = 0; k < mid; k++)
            {
                int temp = matA[i][k] * matB[k][j];
                matC[i][j] += temp;
            }
        }
}

// computation of each element happens in a thread
void th_elements()
{
    nthreads = row * col;
    if(verbose>2)
        printf("t: %d\n",nthreads);
    pthread_t *threads;
    threads = (pthread_t*)malloc(nthreads * sizeof(pthread_t));
    th_create(threads);
}

// computation of each row happens in a thread
void th_rows()
{
    nthreads = mid;
    if(verbose>2)
        printf("t: %d\n",nthreads);
    pthread_t *threads;
    threads = (pthread_t*)malloc(nthreads * sizeof(pthread_t));
    th_create(threads);
}


// Creating/joining all threads
void th_create(pthread_t *threads)
{
    /*
    pthread_create parameters:
                returns the thread id,
                Set to NULL if default thread attributes are used.
                pointer to the function to be threaded,
                pointer to argument of function
    */

    for (int i = 0; i < nthreads; i++)
        pthread_create(&threads[i], NULL, multi, (void*)(i));

    /*
    pthread_join parameters:
                waits for a specific thread to be terminated,
                copies the exit status of target thread if not NULL
    */

    for (int i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);

}


void mat_write(double cpu_time_used, int e)
{
    FILE *fp;
    fp = fopen("output.txt", "a");
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            fprintf(fp,"%d ", matC[i][j]);
        }
        fprintf(fp,"\n");
    }
    fprintf(fp, "END%d	%lf\n\n",e,cpu_time_used);
    fprintf(fp,"\n");
    fclose(fp);
}


int main()
{
    int e = 1;
    clock_t start, end;
    double cpu_time_used;

    mat_read();
    printf("\n");

    start = clock();
    th_elements();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    mat_write(cpu_time_used,e);
    e++;
    printf("END1	%f\n\n",cpu_time_used);

    start = clock();
    th_rows();
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    mat_write(cpu_time_used,e);
    printf("END2	%f\n\n",cpu_time_used);

    if (verbose>0)
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                printf("%d ", matC[i][j]);
            }
            printf("\n");
        }

    return 0;
}


