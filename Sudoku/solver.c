#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>
#include <math.h>

#define N 9          // Size of the Sudoku (9x9)
#define SQRT_N 3     // Square root of N (3 for 9x9)
#define MAX_RUNS 250 // Maximum number of variations
#define MIN_COST (N * N * 3) // Minimum cost for perfect solution
#define FILEPATH "Sudoku_instances/april_5_2025.txt" // Filepath to the instance
#define NAME "april_5_2025" // Name of solution file

// Structure for a cell
typedef struct {
    int row;
    int col;
} Cell;

// Structure for a subgraph (row, column, or block)
typedef struct {
    Cell cells[N];
    int size;
} Subgraph;

// Structure to store all subgraphs
typedef struct {
    Subgraph rows[N];
    Subgraph columns[N];
    Subgraph blocks[N];
    int row_count;
    int col_count;
    int block_count;
} Subgraphs;

// Global variables
int sudoku[N][N];  // Initialized from the file
int cell_matrix[N][N];
int numbers[N];
int tour[N][N];
int best_tour[N][N];
Subgraphs subgraphs;

void parse_sudoku_file(const char *file_path);
void save_sudoku_file(double time);
void generate_cell_matrix();
void generate_subgraphs();
bool validate_sudoku(int sudoku[N][N]);
int calculate_global_cost(int tour[N][N]);
void generate_greedy_tour(int tour[N][N], Cell specific_cell, int specific_number);
int two_opt(int tour[N][N], Cell *cells, int n, int temp_tour[N][N]);
void two_opt_reverse(int tour[N][N], Cell *cells, int n, int temp_tour[N][N]);
int two_opt_and_swap(int tour[N][N]);

// Function to read the Sudoku from a file
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

// Function to save the solved Sudoku
void save_sudoku_file(double time) {
    char output_path[256];
    snprintf(output_path, sizeof(output_path), "Sudoku_results/%s_solution.txt", NAME);

    FILE *file = fopen(output_path, "w");
    if (!file) {
        printf("Error creating file %s\n", output_path);
        exit(1);
    }

    for (int r = 0; r < subgraphs.row_count; r++) {
        for (int c = 0; c < subgraphs.rows[r].size; c++) {
            Cell cell = subgraphs.rows[r].cells[c];
            fprintf(file, "%d ", tour[cell.row][cell.col]);
        }
        fprintf(file, "\n");
    }

    fprintf(file, "\ntotal time %.2f seconds", time);
    fclose(file);
    printf("Result saved in %s\n", output_path);
}

// Function to copy sudoku to cell_matrix
void generate_cell_matrix() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            cell_matrix[i][j] = sudoku[i][j];
        }
    }
}

// Function to generate subgraphs (rows, columns, blocks)
void generate_subgraphs() {
    int row_idx = 0, col_idx = 0, block_idx = 0;

    // Rows
    for (int i = 0; i < N; i++) {
        subgraphs.rows[row_idx].size = 0;
        for (int j = 0; j < N; j++) {
            subgraphs.rows[row_idx].cells[subgraphs.rows[row_idx].size].row = i;
            subgraphs.rows[row_idx].cells[subgraphs.rows[row_idx].size].col = j;
            subgraphs.rows[row_idx].size++;
        }
        row_idx++;
    }
    subgraphs.row_count = row_idx;

    // Columns
    for (int j = 0; j < N; j++) {
        subgraphs.columns[col_idx].size = 0;
        for (int i = 0; i < N; i++) {
            subgraphs.columns[col_idx].cells[subgraphs.columns[col_idx].size].row = i;
            subgraphs.columns[col_idx].cells[subgraphs.columns[col_idx].size].col = j;
            subgraphs.columns[col_idx].size++;
        }
        col_idx++;
    }
    subgraphs.col_count = col_idx;

    // Blocks
    for (int i = 0; i < N; i += SQRT_N) {
        for (int j = 0; j < N; j += SQRT_N) {
            subgraphs.blocks[block_idx].size = 0;
            for (int i2 = i; i2 < i + SQRT_N; i2++) {
                for (int j2 = j; j2 < j + SQRT_N; j2++) {
                    subgraphs.blocks[block_idx].cells[subgraphs.blocks[block_idx].size].row = i2;
                    subgraphs.blocks[block_idx].cells[subgraphs.blocks[block_idx].size].col = j2;
                    subgraphs.blocks[block_idx].size++;
                }
            }
            block_idx++;
        }
    }
    subgraphs.block_count = block_idx;
}

// Function to validate the Sudoku puzzle
bool validate_sudoku(int sudoku[N][N]) {
    // Check rows
    for (int r = 0; r < subgraphs.row_count; r++) {
        int used[N + 1] = {0};
        for (int c = 0; c < subgraphs.rows[r].size; c++) {
            Cell cell = subgraphs.rows[r].cells[c];
            int val = sudoku[cell.row][cell.col];
            if (val != 0) {
                used[val]++;
            }
        }
        printf("{");
        bool first = true;
        for (int i = 1; i <= N; i++) {
            if (used[i] > 0) {
                if (!first) printf(", ");
                printf("%d: %d", i, used[i]);
                first = false;
            }
        }
        printf("}\n");
        for (int i = 1; i <= N; i++) {
            if (used[i] > 1) {
                printf("Invalid sudoku\n");
                return false;
            }
        }
    }

    // Check columns
    for (int c = 0; c < subgraphs.col_count; c++) {
        int used[N + 1] = {0};
        for (int i = 0; i < subgraphs.columns[c].size; i++) {
            Cell cell = subgraphs.columns[c].cells[i];
            int val = sudoku[cell.row][cell.col];
            if (val != 0) {
                used[val]++;
            }
        }
        printf("{");
        bool first = true;
        for (int i = 1; i <= N; i++) {
            if (used[i] > 0) {
                if (!first) printf(", ");
                printf("%d: %d", i, used[i]);
                first = false;
            }
        }
        printf("}\n");
        for (int i = 1; i <= N; i++) {
            if (used[i] > 1) {
                printf("Invalid sudoku\n");
                return false;
            }
        }
    }

    // Check blocks
    for (int b = 0; b < subgraphs.block_count; b++) {
        int used[N + 1] = {0};
        for (int c = 0; c < subgraphs.blocks[b].size; c++) {
            Cell cell = subgraphs.blocks[b].cells[c];
            int val = sudoku[cell.row][cell.col];
            if (val != 0) {
                used[val]++;
            }
        }
        printf("{");
        bool first = true;
        for (int i = 1; i <= N; i++) {
            if (used[i] > 0) {
                if (!first) printf(", ");
                printf("%d: %d", i, used[i]);
                first = false;
            }
        }
        printf("}\n");
        for (int i = 1; i <= N; i++) {
            if (used[i] > 1) {
                printf("Invalid sudoku\n");
                return false;
            }
        }
    }

    printf("Sudoku seems valid.\n");
    return true;
}

// Function to calculate the global cost
int calculate_global_cost(int tour[N][N]) {
    int global_cost = 0;

    // Rows
    for (int r = 0; r < subgraphs.row_count; r++) {
        bool used[N + 1] = {false};
        int unique_count = 0;
        for (int c = 0; c < subgraphs.rows[r].size; c++) {
            Cell cell = subgraphs.rows[r].cells[c];
            int val = tour[cell.row][cell.col];
            if (!used[val]) {
                used[val] = true;
                unique_count++;
            }
        }
        global_cost += unique_count + (N - unique_count) * 2;
    }

    // Columns
    for (int c = 0; c < subgraphs.col_count; c++) {
        bool used[N + 1] = {false};
        int unique_count = 0;
        for (int i = 0; i < subgraphs.columns[c].size; i++) {
            Cell cell = subgraphs.columns[c].cells[i];
            int val = tour[cell.row][cell.col];
            if (!used[val]) {
                used[val] = true;
                unique_count++;
            }
        }
        global_cost += unique_count + (N - unique_count) * 2;
    }

    // Blocks
    for (int b = 0; b < subgraphs.block_count; b++) {
        bool used[N + 1] = {false};
        int unique_count = 0;
        for (int c = 0; c < subgraphs.blocks[b].size; c++) {
            Cell cell = subgraphs.blocks[b].cells[c];
            int val = tour[cell.row][cell.col];
            if (!used[val]) {
                used[val] = true;
                unique_count++;
            }
        }
        global_cost += unique_count + (N - unique_count) * 2;
    }

    return global_cost;
}

// Function to generate an initial greedy solution
void generate_greedy_tour(int tour[N][N], Cell specific_cell, int specific_number) {
    bool used[N + 1];
    int i, j;

    // Initialize tour
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            tour[i][j] = 0;
        }
    }

    for (int r = 0; r < subgraphs.row_count; r++) {
        memset(used, 0, sizeof(used));
        // Fill fixed values
        for (int c = 0; c < subgraphs.rows[r].size; c++) {
            Cell cell = subgraphs.rows[r].cells[c];
            i = cell.row;
            j = cell.col;
            if (cell_matrix[i][j] != 0 && !used[cell_matrix[i][j]]) {
                tour[i][j] = cell_matrix[i][j];
                used[cell_matrix[i][j]] = true;
            }
        }
        // Fill specific cell
        if (specific_cell.row == subgraphs.rows[r].cells[0].row && !used[specific_number]) {
            tour[specific_cell.row][specific_cell.col] = specific_number;
            used[specific_number] = true;
        }
        // Fill remaining cells
        for (int c = 0; c < subgraphs.rows[r].size; c++) {
            Cell cell = subgraphs.rows[r].cells[c];
            i = cell.row;
            j = cell.col;
            if (tour[i][j] == 0) {
                for (int num = 1; num <= N; num++) {
                    if (!used[num]) {
                        tour[i][j] = num;
                        used[num] = true;
                        break;
                    }
                }
            }
        }
    }

    // Print tour
    printf("{");
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            printf("(%d,%d): %d", i, j, tour[i][j]);
            if (i < N - 1 || j < N - 1) printf(", ");
        }
    }
    printf("}\n");
}

// Function two-opt for optimization
int two_opt(int tour[N][N], Cell *cells, int n, int temp_tour[N][N]) {
    memcpy(temp_tour, tour, sizeof(int) * N * N);
    int global_cost = calculate_global_cost(temp_tour);
    bool improved = true;

    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                Cell cell1 = cells[i];
                Cell cell2 = cells[j];
                if (cell_matrix[cell1.row][cell1.col] == 0 &&
                    cell_matrix[cell2.row][cell2.col] == 0 &&
                    temp_tour[cell1.row][cell1.col] != temp_tour[cell2.row][cell2.col]) {
                    int temp = temp_tour[cell1.row][cell1.col];
                    temp_tour[cell1.row][cell1.col] = temp_tour[cell2.row][cell2.col];
                    temp_tour[cell2.row][cell2.col] = temp;
                    int new_global_cost = calculate_global_cost(temp_tour);
                    if (new_global_cost < global_cost) {
                        global_cost = new_global_cost;
                        improved = true;
                        break;
                    } else {
                        temp_tour[cell2.row][cell2.col] = temp_tour[cell1.row][cell1.col];
                        temp_tour[cell1.row][cell1.col] = temp;
                    }
                }
            }
            if (improved) break;
        }
    }
    memcpy(tour, temp_tour, sizeof(int) * N * N);
    return global_cost;
}

// Function two-opt reverse for worsening the tour
void two_opt_reverse(int tour[N][N], Cell *cells, int n, int temp_tour[N][N]) {
    memcpy(temp_tour, tour, sizeof(int) * N * N);
    int global_cost = calculate_global_cost(temp_tour);
    bool improved = true;

    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 1; j < n; j++) {
                Cell cell1 = cells[i];
                Cell cell2 = cells[j];
                if (cell_matrix[cell1.row][cell1.col] == 0 &&
                    cell_matrix[cell2.row][cell2.col] == 0 &&
                    temp_tour[cell1.row][cell1.col] != temp_tour[cell2.row][cell2.col]) {
                    int temp = temp_tour[cell1.row][cell1.col];
                    temp_tour[cell1.row][cell1.col] = temp_tour[cell2.row][cell2.col];
                    temp_tour[cell2.row][cell2.col] = temp;
                    int new_global_cost = calculate_global_cost(temp_tour);
                    if (new_global_cost > global_cost) {
                        global_cost = new_global_cost;
                        improved = true;
                        break;
                    } else {
                        temp_tour[cell2.row][cell2.col] = temp_tour[cell1.row][cell1.col];
                        temp_tour[cell1.row][cell1.col] = temp;
                    }
                }
            }
            if (improved) break;
        }
    }
    memcpy(tour, temp_tour, sizeof(int) * N * N);
}

// Function for refinement with simple swaps
int two_opt_and_swap(int initial_tour[N][N]) {
    int current_tour[N][N];
    int temp_tour[N][N];
    int min_cost = N * N * 3;
    Cell cells[N * N];
    int n = 0;

    // Collect all cells
    for (int r = 0; r < subgraphs.row_count; r++) {
        for (int c = 0; c < subgraphs.rows[r].size; c++) {
            cells[n] = subgraphs.rows[r].cells[c];
            n++;
        }
    }

    // Worsen the tour first
    two_opt_reverse(initial_tour, cells, n, temp_tour);
    memcpy(best_tour, tour, sizeof(int) * N * N);
    int global_cost = two_opt(best_tour, cells, n, temp_tour);
    memcpy(current_tour, best_tour, sizeof(int) * N * N);

    if (global_cost == min_cost) {
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
                    if (cell_matrix[cell1.row][cell1.col] == 0 &&
                        cell_matrix[cell2.row][cell2.col] == 0 &&
                        current_tour[cell1.row][cell1.col] != current_tour[cell2.row][cell2.col]) {
                        int temp = current_tour[cell1.row][cell1.col];
                        current_tour[cell1.row][cell1.col] = current_tour[cell2.row][cell2.col];
                        current_tour[cell2.row][cell2.col] = temp;
                        int new_global_cost = two_opt(current_tour, cells, n, temp_tour);
                        if (new_global_cost < global_cost) {
                            global_cost = new_global_cost;
                            memcpy(best_tour, current_tour, sizeof(int) * N * N);
                            improved = true;
                            printf("%d\n", global_cost);
                            if (global_cost == min_cost) {
                                printf("Found!\n");
                                memcpy(tour, best_tour, sizeof(int) * N * N);
                                return global_cost;
                            }
                        }
                    }
                }
            }
        }
        if (improved) {
            memcpy(current_tour, best_tour, sizeof(int) * N * N);
        }
    }
    memcpy(tour, best_tour, sizeof(int) * N * N);
    return global_cost;
}

// Function for combinatorial optimization
void solve_sudoku(int runs) {
    clock_t start = clock();
    Cell cell_variations[MAX_RUNS];
    int number_variations[MAX_RUNS];
    int counter = 0;

    // Generate variations
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

    int min_cost = N * N * 3;
    int lowest_cost = INT_MAX;

    for (int run = 0; run < counter; run++) {
        Cell specific_cell = cell_variations[run];
        int specific_number = number_variations[run];
        printf("Run: %d\n", run + 1);
        printf("%.2f seconds\n", (double)(clock() - start) / CLOCKS_PER_SEC);
        printf("(%d,%d) %d\n", specific_cell.row, specific_cell.col, specific_number);

        generate_greedy_tour(tour, specific_cell, specific_number);
        int cost = two_opt_and_swap(tour);

        if (cost < lowest_cost) {
            lowest_cost = cost;
            memcpy(best_tour, tour, sizeof(int) * N * N);
        }

        if (cost == min_cost) {
            break;
        }
    }

    // Print the result
    printf("Tour: \n");
    for (int i = 0; i < N; i++) {
        if (i % SQRT_N == 0 && i != 0) {
            printf("- - - - - - - - - - - -\n");
        }
        for (int j = 0; j < N; j++) {
            if (j % SQRT_N == 0 && j != 0) {
                printf("| ");
            }
            printf("%d ", best_tour[i][j]);
        }
        printf("\n");
    }

    // Save the solution
    printf("Saving result...\n");    
    save_sudoku_file((double)(clock() - start)/ CLOCKS_PER_SEC);
    printf("Saved.\n"); 

    printf("\nTotal time: %.2f seconds\n", (double)(clock() - start) / CLOCKS_PER_SEC);
    printf("Lowest cost: %d\n", lowest_cost);
}

// Main function
int main() {
    // Read the Sudoku from the file
    parse_sudoku_file(FILEPATH);

    // Initialize numbers
    for (int i = 0; i < N; i++) {
        numbers[i] = i + 1;
    }

    // Generate cell_matrix and subgraphs
    generate_cell_matrix();
    generate_subgraphs();

    // Validate the Sudoku puzzle
    if (!validate_sudoku(sudoku)) {
        return 1;
    }

    // Print numbers
    printf("numbers: ");
    for (int i = 0; i < N; i++) {
        printf("%d ", numbers[i]);
    }
    printf("\n");

    // Execute optimization
    solve_sudoku(MAX_RUNS);

    return 0;
}
