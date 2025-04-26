#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#define N 9 // Size of sudoku NxN
#define SQRT_N 3 // Square root of N
#define MAX_RUNS 250
#define MIN_COST (N * N * 3) // each row, column and block should have cost N 
#define FILEPATH "Sudoku_instances/march_22_2025.txt"
#define NAME "march_22_2025"

typedef struct {
    int row;
    int col;
} Cell;

typedef struct {
    Cell cells[N];
    int size;
} Subgraph;

typedef struct {
    Subgraph rows[N];
    Subgraph columns[N];
    Subgraph blocks[N];
} Subgraphs;

int sudoku[N][N];
int cell_matrix[N][N];
int numbers[N];
int tour[N][N];
Subgraphs subgraphs;

void parse_sudoku_file(const char *file_path);
void save_sudoku_file(double time);
void generate_cell_matrix();
void generate_subgraphs();
bool validate_sudoku();
int calculate_global_cost(int tour[N][N]);
void generate_greedy_tour(Cell specific_cell, int specific_number);
void two_opt(int tour[N][N], Cell *cells, int n, int *global_cost);
void two_opt_reverse(int tour[N][N], Cell *cells, int n);
int two_opt_and_swap(int initial_tour[N][N], Cell *cells, int n);
void solve_sudoku(int runs);
void print_tour(int tour[N][N]);

void parse_sudoku_file(const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        printf("Error opening file %s\n", file_path);
        exit(1);
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            if (fscanf(file, "%d", &sudoku[i][j]) != 1) {
                printf("Error reading Sudoku from file %s\n", file_path);
                fclose(file);
                exit(1);
            }
        }
    }
    fclose(file);
}

void save_sudoku_file(double time) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "Sudoku_results/%s_solution.txt", NAME);
    FILE *file = fopen(filepath, "w");
    if (!file) {
        printf("Error creating file %s\n", filepath);
        exit(1);
    }

    for (int r = 0; r < N; r++) {
        for (int c = 0; c < subgraphs.rows[r].size; c++) {
            Cell cell = subgraphs.rows[r].cells[c];
            fprintf(file, "%d ", tour[cell.row][cell.col]);
        }
        fprintf(file, "\n");
    }
    fprintf(file, "\ntotal time %.2f seconds", time);
    fclose(file);
    printf("Result saved in %s\n", filepath);
}

void generate_cell_matrix() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cell_matrix[i][j] = sudoku[i][j];
        }
    }
}

void generate_subgraphs() {
    for (int i = 0; i < N; i++) {
        subgraphs.rows[i].size = N;
        for (int j = 0; j < N; j++) {
            subgraphs.rows[i].cells[j].row = i;
            subgraphs.rows[i].cells[j].col = j;
        }
    }

    for (int j = 0; j < N; j++) {
        subgraphs.columns[j].size = N;
        for (int i = 0; i < N; i++) {
            subgraphs.columns[j].cells[i].row = i;
            subgraphs.columns[j].cells[i].col = j;
        }
    }

    int block_idx = 0;
    for (int i = 0; i < N; i += SQRT_N) {
        for (int j = 0; j < N; j += SQRT_N) {
            subgraphs.blocks[block_idx].size = N;
            int idx = 0;
            for (int i2 = i; i2 < i + SQRT_N; i2++) {
                for (int j2 = j; j2 < j + SQRT_N; j2++) {
                    subgraphs.blocks[block_idx].cells[idx].row = i2;
                    subgraphs.blocks[block_idx].cells[idx].col = j2;
                    idx++;
                }
            }
            block_idx++;
        }
    }
}

bool validate_sudoku() {
    for (int s = 0; s < 3; s++) {
        Subgraph *structs = (s == 0) ? subgraphs.rows : (s == 1) ? subgraphs.columns : subgraphs.blocks;
        for (int i = 0; i < N; i++) {
            int used[N + 1] = {0};
            for (int j = 0; j < structs[i].size; j++) {
                Cell cell = structs[i].cells[j];
                int val = sudoku[cell.row][cell.col];
                if (val != 0) {
                    used[val]++;
                }
            }
            printf("{");
            bool first = true;
            for (int num = 1; num <= N; num++) {
                if (used[num] > 0) {
                    if (!first) printf(", ");
                    printf("%d: %d", num, used[num]);
                    first = false;
                }
            }
            printf("}\n");
            for (int num = 1; num <= N; num++) {
                if (used[num] > 1) {
                    printf("Invalid sudoku\n");
                    return false;
                }
            }
        }
    }
    printf("Sudoku seems valid.\n");
    return true;
}

int calculate_global_cost(int tour[N][N]) {
    int global_cost = 0;

    for (int s = 0; s < 3; s++) {
        Subgraph *structs = (s == 0) ? subgraphs.rows : (s == 1) ? subgraphs.columns : subgraphs.blocks;
        for (int i = 0; i < N; i++) {
            bool used[N + 1] = {false};
            int unique_count = 0;
            for (int j = 0; j < structs[i].size; j++) {
                Cell cell = structs[i].cells[j];
                int val = tour[cell.row][cell.col];
                if (!used[val]) {
                    used[val] = true;
                    unique_count++;
                }
            }
            global_cost += unique_count + (N - unique_count) * 2;
        }
    }
    return global_cost;
}

void generate_greedy_tour(Cell specific_cell, int specific_number) {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            tour[i][j] = 0;
        }
    }

    for (int r = 0; r < N; r++) {
        bool used[N + 1] = {false};
        for (int c = 0; c < subgraphs.rows[r].size; c++) {
            Cell cell = subgraphs.rows[r].cells[c];
            if (cell_matrix[cell.row][cell.col] != 0 && !used[cell_matrix[cell.row][cell.col]]) {
                tour[cell.row][cell.col] = cell_matrix[cell.row][cell.col];
                used[cell_matrix[cell.row][cell.col]] = true;
            }
        }
        if (subgraphs.rows[r].cells[0].row == specific_cell.row && !used[specific_number]) {
            tour[specific_cell.row][specific_cell.col] = specific_number;
            used[specific_number] = true;
        }
        for (int c = 0; c < subgraphs.rows[r].size; c++) {
            Cell cell = subgraphs.rows[r].cells[c];
            if (tour[cell.row][cell.col] == 0) {
                for (int num = 1; num <= N; num++) {
                    if (!used[num]) {
                        tour[cell.row][cell.col] = num;
                        used[num] = true;
                        break;
                    }
                }
            }
        }
    }
    print_tour(tour);
}

void print_tour(int tour[N][N]) {
    printf("{");
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            printf("(%d,%d): %d", i, j, tour[i][j]);
            if (i < N - 1 || j < N - 1) printf(", ");
        }
    }
    printf("}\n");
}

void two_opt(int tour[N][N], Cell *cells, int n, int *global_cost) {
    *global_cost = calculate_global_cost(tour);
    bool improved = true;

    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                Cell cell1 = cells[i];
                Cell cell2 = cells[j];
                if (cell_matrix[cell1.row][cell1.col] == 0 && cell_matrix[cell2.row][cell2.col] == 0 &&
                    tour[cell1.row][cell1.col] != tour[cell2.row][cell2.col]) {
                    int temp = tour[cell1.row][cell1.col];
                    tour[cell1.row][cell1.col] = tour[cell2.row][cell2.col];
                    tour[cell2.row][cell2.col] = temp;
                    int new_cost = calculate_global_cost(tour);
                    if (new_cost < *global_cost) {
                        *global_cost = new_cost;
                        improved = true;
                    } else {
                        tour[cell2.row][cell2.col] = tour[cell1.row][cell1.col];
                        tour[cell1.row][cell1.col] = temp;
                    }
                }
            }
        }
    }
}

void two_opt_reverse(int tour[N][N], Cell *cells, int n) {
    int global_cost = calculate_global_cost(tour);
    bool improved = true;

    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                Cell cell1 = cells[i];
                Cell cell2 = cells[j];
                if (cell_matrix[cell1.row][cell1.col] == 0 && cell_matrix[cell2.row][cell2.col] == 0 &&
                    tour[cell1.row][cell1.col] != tour[cell2.row][cell2.col]) {
                    int temp = tour[cell1.row][cell1.col];
                    tour[cell1.row][cell1.col] = tour[cell2.row][cell2.col];
                    tour[cell2.row][cell2.col] = temp;
                    int new_cost = calculate_global_cost(tour);
                    if (new_cost > global_cost) {
                        global_cost = new_cost;
                        improved = true;
                    } else {
                        tour[cell2.row][cell2.col] = tour[cell1.row][cell1.col];
                        tour[cell1.row][cell1.col] = temp;
                    }
                }
            }
        }
    }
}

// For decision problems (NP-complete) generating a new local minimum from a pertubed local minimum have better results.
int two_opt_and_swap(int initial_tour[N][N], Cell *cells, int n) {
    int best_tour[N][N];
    int current_tour[N][N];
    int global_cost;

    memcpy(best_tour, initial_tour, sizeof(int) * N * N);
    two_opt_reverse(best_tour, cells, n);
    two_opt(best_tour, cells, n, &global_cost);
    memcpy(current_tour, best_tour, sizeof(int) * N * N);

    if (global_cost == MIN_COST) {
        printf("Found!\n");
        memcpy(tour, best_tour, sizeof(int) * N * N);
        return global_cost;
    }

    bool improved = true;
    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1; i++) {
            for (int j = 0; j < n; j++) {
                if (i != j) {
                    Cell cell1 = cells[i];
                    Cell cell2 = cells[j];
                    if (cell_matrix[cell1.row][cell1.col] == 0 && cell_matrix[cell2.row][cell2.col] == 0 &&
                        current_tour[cell1.row][cell1.col] != current_tour[cell2.row][cell2.col]) {
                        int temp = current_tour[cell1.row][cell1.col];
                        current_tour[cell1.row][cell1.col] = current_tour[cell2.row][cell2.col];
                        current_tour[cell2.row][cell2.col] = temp;
                        int new_cost;
                        two_opt(current_tour, cells, n, &new_cost);
                        if (new_cost < global_cost) {
                            memcpy(best_tour, current_tour, sizeof(int) * N * N);
                            global_cost = new_cost;
                            improved = true;
                            printf("%d\n", global_cost);
                            if (global_cost == MIN_COST) {
                                printf("Found!\n");
                                memcpy(tour, best_tour, sizeof(int) * N * N);
                                return global_cost;
                            }
                            break;
                        }
                    }
                }
            }
            if (improved) break;
        }
    }
    print_tour(best_tour);
    memcpy(tour, best_tour, sizeof(int) * N * N);
    return global_cost;
}

void solve_sudoku(int runs) {
    clock_t start = clock();
    Cell cell_variations[MAX_RUNS];
    int number_variations[MAX_RUNS];
    int counter = 0;

    for (int i = 0; i < N && counter < runs; i++) {
        for (int j = 0; j < N && counter < runs; j++) {
            if (cell_matrix[i][j] == 0) {
                for (int num = 1; num <= N && counter < runs; num++) {
                    cell_variations[counter].row = i;
                    cell_variations[counter].col = j;
                    number_variations[counter] = num;
                    counter++;
                }
            }
        }
    }

    int lowest_cost = INT_MAX;
    Cell cells[N * N];
    int n = 0;
    for (int r = 0; r < N; r++) {
        for (int c = 0; c < subgraphs.rows[r].size; c++) {
            cells[n] = subgraphs.rows[r].cells[c];
            n++;
        }
    }

    for (int run = 0; run < counter; run++) {
        printf("Current run: %d\n", run + 1);
        printf("%.2f seconds\n", (double)(clock() - start) / CLOCKS_PER_SEC);
        printf("(%d,%d) %d\n", cell_variations[run].row, cell_variations[run].col, number_variations[run]);
        generate_greedy_tour(cell_variations[run], number_variations[run]);
        int cost = two_opt_and_swap(tour, cells, n);
        if (cost < lowest_cost) {
            lowest_cost = cost;
        }
        if (cost == MIN_COST) {
            break;
        }
    }

    printf("Saving result...\n");
    save_sudoku_file((double)(clock() - start) / CLOCKS_PER_SEC);
    printf("Saved.\n");
    printf("Total time: %.2f seconds\n", (double)(clock() - start) / CLOCKS_PER_SEC);
    printf("Lowest cost: %d\n", lowest_cost);
}

int main() {
    parse_sudoku_file(FILEPATH);
    for (int i = 0; i < N; i++) {
        numbers[i] = i + 1;
    }
    generate_cell_matrix();
    generate_subgraphs();
    if (!validate_sudoku()) {
        return 1;
    }
    solve_sudoku(MAX_RUNS);
    return 0;
}
