#include <time.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define X 1
#define EMPTY 10
#define NO_WINNER 20

#define N 4
#define M N

typedef unsigned char symbol_next;

typedef struct grid {
	symbol_next m[N][M];
	unsigned short n_empty;
} grid_t;

typedef struct move {
	unsigned short i, j;
} move_t;


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

void Show_grid(grid_t* grid) {
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

int move(grid_t* grid, symbol_next symbol, int depth, int alpha, int beta) {
	int n, i;
	move_t* max_move;
	int score = calculate_score(grid, depth, symbol);

	if(score != 0) {
		return score;
	}

	move_t** play = get_all_possible_play(grid, symbol, &n);
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
	
	grid_t* grid = (grid_t*) malloc(sizeof(grid_t));
	for(i = 0; i < N; i++) {
		for(j = 0; j < M; j++) {
			grid->m[i][j] = EMPTY;
		}
	}
	grid->n_empty = N * M;
	
	symbol_next result;
	symbol_next current_symbol = X;

	move_t m;
 	m.i = 1;
	m.j = 3;
	
	Show_grid(grid);
	
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
	Show_grid(grid);
	
	if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) { perror("clock gettime");}		
	time = (stop.tv_sec - start.tv_sec)+ (double)(stop.tv_nsec - start.tv_nsec)/1e9;
	printf("Execution time = %f sec,\n", time);
	
	printf("Winner: %i\n:", (int) result);

	return 0;
}
