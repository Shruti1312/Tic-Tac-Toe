#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define X 1
#define EMPTY 10
#define NO_WINNER 20

#define N 4
#define M 4

typedef unsigned char symbol_next;

typedef struct grid {
	symbol_next m[N][M];
	unsigned short n_empty;
} grid_t;

typedef struct move {
	unsigned short i, j;
} move_t;

int move(grid_t* grid, symbol_next symbol, int depth, int alpha, int beta);

int global_n, next_free_move;
move_t** global_play;
int global_max_score;
move_t* global_max_move;
pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
grid_t* global_grid;
symbol_next global_symbol;

int NUM_THREADS = 1;

void put_symbol(grid_t* grid, symbol_next symbol, move_t* move) {
	grid->m[move->i][move->j] = symbol;
	grid->n_empty --;
}

symbol_next winner(grid_t* b) {
	int i, j;
	symbol_next sym;
	int equal;

	for(i = 0; i < N; i++) {
		equal = 1;
		sym = b->m[i][0];
		if(sym != EMPTY) {
			for(j = 1; j < M; j++) {
				if(b->m[i][j] != sym) {
					equal = 0;
					break;
				}
			}

			if(equal == 1) {
				return sym;
			}
		}
	
	}

	for(i = 0; i < M; i++) {
		equal = 1;
		sym = b->m[0][i];
		if(sym != EMPTY) {
			for(j = 1; j < N; j++) {
				if(b->m[j][i] != sym) {
					equal = 0;
					break;
				}
			}

			if(equal == 1) {
				return sym;
			}
		}
	
	}
		
	equal = 1;
	sym = b->m[0][0];
	if(sym != EMPTY) {
		for(i = 1; i < N; i++) {
			if(b->m[i][i] != sym) {
				equal = 0;
				break;
			}
		}

		if(equal == 1) {
			return sym;
		}
	}

	equal = 1;
	sym = b->m[0][M-1];
	if(sym != EMPTY) {
		for(i = 1; i < N; i++) {
			if(b->m[i][M-i-1] != sym) {
				equal = 0;
				break;
			}
		}

		if(equal == 1) {
			return sym;
		}
	}

	if(b->n_empty == 0) {
		return NO_WINNER;
	}

	return EMPTY;
}

void show_grid(grid_t* grid) {
	int i, j;
	for(i = 0; i < N; i++) {
		for(j = 0; j < M; j++) {
			if(grid->m[i][j] == X) {
				printf("X ");
			} else if(grid->m[i][j] == 0) {
				printf("0 ");
			} else {
				printf("- ");
			}
		}
		printf("\n");
	}
}

move_t** get_all_possible_play(grid_t* grid, symbol_next symbol, int* n) {
	int i,j;

	move_t** list = (move_t**) malloc(grid->n_empty * sizeof(move_t*));
	*n = 0;

	for(i = 0; i < N; i++) {
		for(j = 0; j < M; j++) {
			if(grid->m[i][j] == EMPTY) {
				list[(*n)] = (move_t*) malloc(sizeof(move_t));
				list[(*n)]->i = i;
				list[(*n)]->j = j;
				(*n) ++;
			}
		}

	}
	return list;
}

symbol_next opponent_mark(symbol_next symbol) {
	return 1 - symbol;
}

grid_t* clone_grid(grid_t* b) {
	int i, j;
	grid_t* grid = (grid_t*) malloc(sizeof(grid_t));
	grid->n_empty = b->n_empty;
	for(i = 0; i < N; i++) {
		for(j = 0; j < M; j++) {
			grid->m[i][j] = b->m[i][j];
		}
	}

	return grid;
}

int calculate_score(grid_t* grid, int depth, symbol_next symbol) {
	symbol_next result = winner(grid);
	
	if(result == symbol) {
		return N * M + 10 - depth;
	} else if(result != EMPTY && result != NO_WINNER) {
		return -(N * M) - 10 + depth;
	} else if(result == NO_WINNER) {
		return 1;
	}

	return 0;
}

void* thread_compute_func(void* arg) {
	int thread_id = ((long int) arg);
	int i, score;
	int max_score;
	
	while(1) {

		pthread_mutex_lock(&mutex1);

		if(next_free_move >= global_n) {
			pthread_mutex_unlock(&mutex1);
			break;
		}

		i = next_free_move;
		next_free_move++;
		pthread_mutex_unlock(&mutex1);

		printf("thread %i computes move %i\n", thread_id, i);

		grid_t* grid = clone_grid(global_grid);
		put_symbol(grid, global_symbol, global_play[i]);
		
		pthread_mutex_lock(&mutex2);
		max_score = global_max_score;
		pthread_mutex_unlock(&mutex2);
		
		score = -move(grid, opponent_mark(global_symbol), 1, -9999, -max_score);

		free(grid);

		pthread_mutex_lock(&mutex2);
		if(score > global_max_score) {
			global_max_score = score;
			global_max_move = global_play[i];
		}
		pthread_mutex_unlock(&mutex2);
	}
	printf("thread %i finished\n", thread_id);
	return 0;
}

int move(grid_t* grid, symbol_next symbol, int depth, int alpha, int beta) {
	int n, i;
	int score = calculate_score(grid, depth, symbol);
	move_t* max_move;

	if(score != 0) {
		return score;
	}

	move_t** play = get_all_possible_play(grid, symbol, &n);
	if(depth == 0) {
		pthread_t threads[NUM_THREADS];
		global_play = play;
		global_max_score = -9999;
		next_free_move = 0;
		global_grid = grid;
		global_symbol = symbol;
		global_n = n;

		for (i = 0; i < NUM_THREADS; i++) {
			pthread_create(&threads[i], NULL, thread_compute_func, (void*) (long int)i);
		}

		for (i = 0; i < NUM_THREADS; i++) {
			pthread_join(threads[i], NULL);
		}
	
		printf("join completed\n");

		max_move = global_max_move;
		alpha = global_max_score;
	} else {
		for(i = 0; i < n; i++) {
			put_symbol(grid, symbol, play[i]);
			score = -move(grid, opponent_mark(symbol), depth + 1, -beta, -alpha);
			grid->m[play[i]->i][play[i]->j] = EMPTY;
			grid->n_empty ++;

			if(score > alpha) {
				alpha = score;
				max_move = play[i];
			}

			if(alpha >= beta) {
				break;
			}
		}
	}

	if(depth == 0) {
		put_symbol(grid, symbol, max_move);
	}

	for(i = 0; i < n; i++) {
		free(play[i]);
	}

	free(play);
	return alpha;
}

int main(int argc, char* argv[]) {	
	struct timespec start, stop;
	double time;
	int i, j;
	
	/* Creating tic_tae_toe grid layout */
	grid_t* grid = (grid_t*) malloc(sizeof(grid_t));
	for(i = 0; i < N; i++) {
		for(j = 0; j < M; j++) {
			grid->m[i][j] = EMPTY;
		}
	}
	grid->n_empty = N * M;
	
	symbol_next result;
	symbol_next current_symbol = X;

	NUM_THREADS = atoi(argv[1]);
	printf("Running for Number of Threads =%d\n", NUM_THREADS);
	show_grid(grid);
	
	pthread_mutex_init ( &mutex1, NULL);
	pthread_mutex_init ( &mutex2, NULL);
	if( clock_gettime(CLOCK_REALTIME, &start) == -1) { perror("clock gettime");}
	while(1) {
		printf("Player %i to move next\n", (int) current_symbol);
		move(grid, current_symbol, 0, -9999, 9999);
		result = winner(grid);
		if(result != EMPTY) {
			break;
		}
		current_symbol = 1 - current_symbol;
	}
	show_grid(grid);
	
	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	printf("Execution time = %f sec,\n", time);
	
	printf("Winner: %i\n:", (int) result);

	return 0;
}
