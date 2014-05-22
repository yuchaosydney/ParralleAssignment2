#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
/* 
   white = 0, red = 1, blue = 2, 
   red or blue just moved in = 3 and 
   red or blue (in the first row or column) just moved out = 4
*/
typedef struct {
  pthread_mutex_t		count_lock;		/* mutex semaphore for the barrier */
  pthread_cond_t		ok_to_proceed;	/* condition variable for leaving */
  int count;
} mylib_barrier_t;

void mylib_barrier_init(mylib_barrier_t *b) {
  b -> count = 0;
  pthread_mutex_init(&(b -> count_lock), NULL);
  pthread_cond_init(&(b -> ok_to_proceed), NULL);
}

void mylib_barrier(mylib_barrier_t *b,int num_threads) {	  

	pthread_mutex_lock(&(b->count_lock));
	printf("barrier.count %d \n",b->count);
	(b->count)++;
	if ((b->count) == num_threads) {
		b->count = 0;
		pthread_cond_broadcast(&(b->ok_to_proceed));
	}
	else
	while (pthread_cond_wait(&(b->ok_to_proceed), &(b->count_lock)) != 0);
	pthread_mutex_unlock(&(b->count_lock));
}

void mylib_barrier_destroy(mylib_barrier_t *b) {  
 
}
void *Thread(void *tid) {
	int *myid;
	myid = (int *) tid;
}


/* barrier */
mylib_barrier_t	barrier;


int main(int argc, char * argv[])
{
	
	int n = 6;
	int grid[n][n]; 	/* grid[row][col] */
	bool finished = false;
	int n_itrs = 0;
	int redcount, bluecount;
	int red_number = (n * n) / 3, blue_number = (n * n) / 3;
	int i = 0, j = 0, k = 0;
	int MAX_ITRS = 50000000;
	int thread_num = 5, tids[thread_num];
	
	int row = 0, column = 0;
	/*tile computation*/
	int t = 3;
	float percentage = 0.6;
	int red_number_tile = 0, blue_number_tile = 0;
	
	/*initiate board variables*/
	int put_row = 0, put_column = 0;
	int each_row_count = 0, each_column_count = 0;
	bool check_loop = true;
	int flag = 0;

	
	/* thread ids and attributes */
	pthread_t workerid[thread_num];
	pthread_attr_t attr;
	
	 /* make threads joinable */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	/* initialize barrier */
	mylib_barrier_init(&barrier);
	
	/*generate the grid randomly*/
	for(row = 0; row < n; row ++) {
		 for(column = 0; column < n; column ++){
			//printf("%d ", grid[row][column]);
			grid[row][column] = 0;
		}
	  }
	
	/*initiate board*/	
	for(row = 0;row < n; row ++) {
		for(each_row_count = 0; each_row_count < red_number / n; each_row_count ++) {
			grid[row][rand() % n] = 1;
		}
	}
	for(row = 0;row < n; row ++) {
		for(each_row_count = 0; each_row_count < blue_number / n; each_row_count ++) {
			while(true) {
				column = rand() % n;
				if(grid[row][column] != 0) {
					continue;
				}else {
					grid[row][column] = 2;
					break;
				}
			}
		}
	}
	  
	 
	while (!finished){
	   n_itrs++; 

	   /* red color movement */
	   for (i = 0; i < n; i++){
		if (grid[i][0] == 1 && grid[i][1] == 0){
			grid[i][0] = 4;
			grid[i][1] = 3;
		}
		for (j = 1; j < n; j++){
			if (grid[i][j] == 1 && grid[i][(j+1)%n] == 0){
				grid[i][j] = 0;
				grid[i][(j+1)%n] = 3;
			}
			else if (grid[i][j] == 3)
				grid[i][j] = 1;
		}
		if (grid[i][0] == 3)
			grid[i][0] = 1;
		else if (grid[i][0] == 4)
			grid[i][0] = 0;
	   }

	   /* blue color movement */
	   for (j = 0; j < n; j++){
		if (grid[0][j] == 2 && grid[1][j] == 0){
			grid[0][j] = 4;
			grid[1][j] = 3;
		}
		for (i = 1; i < n; i++){
			if (grid[i][j] == 2 && grid[(i+1)%n][j]==0){
				grid[i][j] = 0;
				grid[(i+1)%n][j] = 3;
			}
			else if (grid[i][j] == 3)
				grid[i][j] = 2;
		}
		if (grid[0][j] == 3)
			grid[0][j] = 2;
		else if (grid[0][j] == 4)
			grid[0][j] = 0;
	  }
		
	  /*print out each step result*/
	  /*.print out result.*/
		for(row = 0; row < n; row ++) {
			 for(column = 0; column < n; column ++){
				printf("%d ", grid[row][column]);
			}
			printf("\n");
		}
		printf("-------------\n");
	  
	  /* count the number of red and blue in each tile and check if the computation can be terminated*/
		for(i  = 0;i < (n / t);i ++) {
			for(j = 0;j < (n / t); j ++) {
				//query each tile
				red_number_tile = 0;
				blue_number_tile = 0;
				for(row = i * t;row < t * (i + 1); row ++) {
					for(column = i * t; column < t * (i + 1); column ++) {
						if(grid[row][column] == 1)
							red_number_tile ++;
						else if(grid[row][column] == 2)
							blue_number_tile ++;
					}
				}
				
				if(((float)red_number_tile / (float)(t * t)) >= percentage || ((float)blue_number_tile / (float)(t * t)) >= percentage) {
					printf("i:j-----%d:%d\n", i, j);
					finished = true;
					break;
				}	
			}
		}
	  /*...*/
	  
	}
	
	//return 0;
}