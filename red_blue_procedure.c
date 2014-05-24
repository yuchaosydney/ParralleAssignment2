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

int** grid; 
int row_column_number;
bool finished;
int n;
int thread_num;
int running_thread_num;
int n_itrs;
pthread_mutex_t global_grid_lock;
/*tile computation*/
int t;
float percentage;

struct thread_data
{
	int	thread_id;
	int iter_times;
};

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
	//printf("barrier.count %d \n",b->count);
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
	pthread_cond_destroy (&(b->ok_to_proceed));
	pthread_mutex_destroy(&(b->count_lock));
}

struct thread_data *thread_data_array;
mylib_barrier_t	barrier;

void *do_work(void *tid) {

	FILE *out_file;
	int myid;
	int row, column;
	int i,j,k;
	int iter_times;
	struct thread_data *my_data;
	int row_or_column;
	int red_number_tile = 0, blue_number_tile = 0;
	
	my_data = (struct thread_data *) tid;
	
	sleep(1);
	
	//printf("-----Thread---%d---%d--\n",my_data->thread_id,my_data->row_column_number);
	row_or_column = my_data->thread_id;
	myid = my_data->thread_id;
	iter_times = my_data->iter_times;
	
	if(myid == 0)
	out_file = fopen("result.txt", "a"); // write only
	
	while(!finished) {
		for(k = 0;k < iter_times;k ++) {
		pthread_mutex_lock(&global_grid_lock);
			//n_itrs++;
			/* red color movement */
			if (grid[row_or_column + k][0] == 1 && grid[row_or_column + k][1] == 0){
				grid[row_or_column + k][0] = 4;
				grid[row_or_column + k][1] = 3;
				
			}
			for (j = 1; j < n; j++){
				if (grid[row_or_column + k][j] == 1 && grid[row_or_column + k][(j+1)%n] == 0){
					grid[row_or_column + k][j] = 0;
					grid[row_or_column + k][(j+1)%n] = 3;
				}
				else if (grid[row_or_column + k][j] == 3)
					grid[row_or_column + k][j] = 1;
			}
			if (grid[row_or_column + k][0] == 3)
				grid[row_or_column + k][0] = 1;
			else if (grid[row_or_column + k][0] == 4)
				grid[row_or_column + k][0] = 0;
			
			printf("Thread %d row moving print out.\n",row_or_column);
			/*print out each step result*/
			/*.print out result.*/
			
			for(row = 0; row < n; row ++) {
				 for(column = 0; column < n; column ++){
					printf(" %d ", grid[row][column]);
				}
				printf("\n");
			}
			printf("------row-------\n");
			pthread_mutex_unlock(&global_grid_lock);
		}
		
			printf("Thread %d is waiting for others.\n",row_or_column);
			//barrier here
			 mylib_barrier(&barrier, running_thread_num);
			
			
		for(k = 0;k < iter_times;k ++) {
		pthread_mutex_lock(&global_grid_lock);
			/* blue color movement */
		  
			if (grid[0][row_or_column + k] == 2 && grid[1][row_or_column + k] == 0){
				grid[0][row_or_column + k] = 4;
				grid[1][row_or_column + k] = 3;
			}
			for (i = 1; i < n; i++){
				if (grid[i][row_or_column + k] == 2 && grid[(i+1)%n][row_or_column + k]==0){
					grid[i][row_or_column + k] = 0;
					grid[(i+1)%n][row_or_column + k] = 3;
				}
				else if (grid[i][row_or_column + k] == 3)
					grid[i][row_or_column + k] = 2;
			}
			if (grid[0][row_or_column + k] == 3)
				grid[0][row_or_column + k] = 2;
			else if (grid[0][row_or_column + k] == 4)
				grid[0][row_or_column + k] = 0;
			
			printf("Thread %d column moving print out.\n",row_or_column);
			/*print out each step result*/
			/*.print out result.*/
			
			for(row = 0; row < n; row ++) {
				 for(column = 0; column < n; column ++){
					printf(" %d ", grid[row][column]);
				}
				printf("\n");
			}
			printf("------column-------\n");
			pthread_mutex_unlock(&global_grid_lock);
		}
			/*barrier here*/
			//barrier here
			printf("Thread %d is waiting for others.\n",row_or_column);
			mylib_barrier(&barrier, running_thread_num);
			 
			/*thread 0 print out each step result*/
			if(myid == 0) {
			 // test for files not existing. 
			 if (out_file == NULL) 
				{   
				  printf("Error! Could not open file\n"); 
				  exit(-1); // must include stdlib.h 
				} 
				/*.print out result.*/
				for(row = 0; row < n; row ++) {
					 for(column = 0; column < n; column ++){
						// write to file
						fprintf(out_file, " %d", grid[row][column]); // write to file
					}
					fprintf(out_file, "\n"); // write to file
				}
				fprintf(out_file, "\n\n\n\n\n"); // write to file
			}

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
						printf("i:j-----%d:%d\n",i, j);
						finished = true;
						break;
					}	
				}
			}
	}	
	if(myid == 0)
	fclose(out_file);
	printf("Thread %d is going to die......\n",row_or_column);
	pthread_exit(NULL);
}




int main(int argc, char * argv[])
{
	int redcount, bluecount;
	
	int red_number, blue_number;
	int i = 0, j = 0, k = 0;
	int MAX_ITRS = 50000000;

	int row = 0, column = 0;
	
	
	/*initiate board variables*/
	int put_row = 0, put_column = 0;
	int each_row_count = 0, each_column_count = 0;
	bool check_loop = true;
	int flag = 0;
	
	
	
	/* thread ids and attributes */
	//pthread_t workerid[thread_num];
	pthread_attr_t attr;
	pthread_t threads[thread_num];
	//struct thread_data *thread_data_array;
	thread_data_array = (struct thread_data *) calloc(thread_num , sizeof (struct thread_data));
	/* make threads joinable */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	pthread_mutex_init(&global_grid_lock, NULL);
	/*tile computation*/
	t = 3;
	percentage = 0.6;
	thread_num = 9;
	n = 6;
	red_number = (n * n) / 3;
	blue_number = (n * n) / 3;
	//int grid[n][n]; 	/* grid[row][col] */
	finished = false;
	n_itrs = 0;
	
	/* initialize barrier */
	mylib_barrier_init(&barrier);
	
	/*generate the grid randomly*/
	grid = (int **) malloc (n * sizeof (int));
    for (i = 0; i < n; i++) {
        grid[i] = (int *)malloc (n * sizeof (int));
    }
	
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
	
	/*.print out result.*/
		for(row = 0; row < n; row ++) {
			 for(column = 0; column < n; column ++){
				printf(" %d ", grid[row][column]);
			}
			printf("\n");
		}
		printf("-------------\n");
	/*create threads to move colours*/
	if(n <= thread_num) {
		for (i=0; i < n; i++) {
			thread_data_array[i].thread_id = i;
			thread_data_array[i].iter_times = 1;
			//printf("creating thread %d\n", i);
			pthread_create(&threads[i], NULL, do_work, (void *) &thread_data_array[i]);  
		}
		running_thread_num = n;
	}else{
		for (i=0; i < thread_num; i++) {
			if(i == thread_num - 1) {
				//final thread receive the last multiple jobs
				thread_data_array[i].thread_id = i;
				thread_data_array[i].iter_times = n - thread_num + 1;
				//printf("creating thread %d\n", i);
				pthread_create(&threads[i], NULL, do_work, (void *) &thread_data_array[i]);  
			}else{
				thread_data_array[i].thread_id = i;
				thread_data_array[i].iter_times = 1;
				//printf("creating thread %d\n", i);
				pthread_create(&threads[i], NULL, do_work, (void *) &thread_data_array[i]);  
			}
			running_thread_num = thread_num;
		}
	}
	
		
	for (i=0; i<thread_num; i++) {
		pthread_join(threads[i], NULL);
	}
		
	
	mylib_barrier_destroy(&barrier); /* destroy barrier object */
	pthread_exit (NULL);
	return 0;
}