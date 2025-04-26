#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <limits.h>

#define MAX_NODES 1000
#define MAX_RUNS 5
#define FILEPATH "TSP_instances/xqf131.tsp"
#define OPT_FILEPATH "TSP_instances/xqf131.tour" // Optional set to NULL if theres none.
#define NAME "xqf131"

typedef struct {
    long long int x, y; // int may be small for some coordinates
} Point;

typedef struct {
    int* tour;
    int dist;
} TourResult;

typedef struct {
    int node;
    int distance;
} Neighbor;

Point* parse_tsp_file(const char* file_path, int* num_points);
int* parse_tour_file(const char* file_path, int* num_points);
int create_tour_file();
void update_tour_file(const int* tour, int num_nodes, int dist, double time, int tourfile_number);
int** pre_process(Point* points, int num_points);
int calculate_distance(Point p1, Point p2);
int calculate_tour_length(const int* tour, int n, int** distances);
int calculate_tour_distance(Point* points, const int* tour, int n);
TourResult two_opt_swap(int** distances, const int* initial_tour, int n);
int* two_opt_reverse(int** distances, const int* initial_tour, int n);
TourResult two_opt_and_swap(int** distances, const int* initial_tour, int n);
int* nearest_neighbor(int** distances, int n, int initial_point);
void free_distances(int** distances, int num_points);
int compare_neighbors(const void* a, const void* b);

int compare_neighbors(const void* a, const void* b) {
    Neighbor* na = (Neighbor*)a;
    Neighbor* nb = (Neighbor*)b;
    if (na->distance != nb->distance) return na->distance - nb->distance;
    return na->node - nb->node;
}

Point* parse_tsp_file(const char* file_path, int* num_points) {
    FILE* file = fopen(file_path, "r");
    if (!file) {
        printf("Error: Unable to open file %s\n", file_path);
        return NULL;
    }

    Point* points = malloc(MAX_NODES * sizeof(Point));
    if (!points) {
        fclose(file);
        return NULL;
    }

    char line[256];
    bool node_section = false;
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "NODE_COORD_SECTION")) {
            node_section = true;
            continue;
        }
        if (strstr(line, "EOF") || line[0] == '\0') break;

        if (node_section) {
            int id;
            float x, y;
            if (sscanf(line, "%d %f %f", &id, &x, &y) == 3) {
                if (count >= MAX_NODES) {
                    printf("Error: Too many points\n");
                    free(points);
                    fclose(file);
                    return NULL;
                }
                points[count].x = round(x);
                points[count].y = round(y);
                count++;
            }
        }
    }

    *num_points = count;
    fclose(file);
    return points;
}

int* parse_tour_file(const char* file_path, int* num_points) {
    FILE* file = fopen(file_path, "r");
    if (!file) {
        return NULL;
    }

    int* tour = malloc(MAX_NODES * sizeof(int));
    if (!tour) {
        fclose(file);
        return NULL;
    }

    char line[256];
    bool tour_section = false;
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "TOUR_SECTION")) {
            tour_section = true;
            continue;
        }
        if (strcmp(line, "-1\n") == 0) break;

        if (tour_section) {
            int point;
            if (sscanf(line, "%d", &point) == 1) {
                if (count >= MAX_NODES) {
                    printf("Error: Too many tour points\n");
                    free(tour);
                    fclose(file);
                    return NULL;
                }
                tour[count++] = point - 1; // 1-based to 0-based
            }
        }
    }

    fclose(file);
    return tour;
}

int create_tour_file() {
    int counter = 1;
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "TSP_results/%s_%d.tour", NAME, counter); 

    while (fopen(filepath, "r")) {
        counter++;
        snprintf(filepath, sizeof(filepath), "TSP_results/%s_%d.tour", NAME, counter);
    }

    FILE* f = fopen(filepath, "w");
    if (!f) {
        printf("Error: Unable to create file %s\n", filepath);
        return 0;
    }

    fclose(f);
    printf("Tour file created in %s\n", filepath);

    return counter;
}

void update_tour_file(const int* tour, int num_nodes, int dist, double time, int tourfile_number) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "TSP_results/%s_%d.tour", NAME, tourfile_number);

    FILE* f = fopen(filepath, "w");
    if (!f) {
        printf("Error: Unable to update file %s\n", filepath);
        return;
    }

    fprintf(f, "NAME: %s\n", NAME);
    fprintf(f, "COMMENT: Tour length %d, total time %.2f seconds\n", dist, time);
    fprintf(f, "TYPE: TOUR\n");
    fprintf(f, "DIMENSION: %d\n", num_nodes);
    fprintf(f, "TOUR_SECTION\n");
    for (int i = 0; i < num_nodes; i++) fprintf(f, "%d\n", tour[i] + 1);
    fprintf(f, "-1\nEOF\n");
    fclose(f);
    printf("Tour updated in %s\n", filepath);
}

int calculate_distance(Point p1, Point p2) {
    return round(sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y)));
}

int** pre_process(Point* points, int num_points) {
    int** distances = malloc(num_points * sizeof(int*));
    Neighbor* temp = malloc(num_points * sizeof(Neighbor));
    for (int i = 0; i < num_points; i++) {
        distances[i] = malloc(num_points * sizeof(int));
    }

    for (int i = 0; i < num_points; i++) {
        int temp_count = 0;
        for (int j = 0; j < num_points; j++) {
            if (i == j) {
                distances[i][j] = 0;
                continue;
            }
            temp[temp_count].node = j;
            temp[temp_count].distance = calculate_distance(points[i], points[j]);
            temp_count++;
        }
        qsort(temp, temp_count, sizeof(Neighbor), compare_neighbors);
        for (int k = 0; k < temp_count; k++) {
            distances[i][temp[k].node] = temp[k].distance;
        }
    }
    free(temp);
    return distances;
}

int calculate_tour_length(const int* tour, int n, int** distances) {
    int length = 0;
    for (int i = 0; i < n; i++) {
        int current = tour[i];
        int next = tour[(i + 1) % n];
        length += distances[current][next];
    }
    return length;
}

int calculate_tour_distance(Point* points, const int* tour, int n) {
    int total = 0;
    for (int i = 0; i < n - 1; i++) {
        total += calculate_distance(points[tour[i]], points[tour[i + 1]]);
    }
    total += calculate_distance(points[tour[n - 1]], points[tour[0]]);
    return total;
}

TourResult two_opt_swap(int** distances, const int* initial_tour, int n) {
    int* best_tour = malloc(n * sizeof(int));
    memcpy(best_tour, initial_tour, n * sizeof(int));
    int shortest_dist = calculate_tour_length(best_tour, n, distances);
    bool improved = true;

    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 2; j < n; j++) {
                int a = best_tour[i], b = best_tour[(i + 1) % n];
                int c = best_tour[j], d = best_tour[(j + 1) % n];
                int current_dist = distances[a][b] + distances[c][d];
                int new_dist = distances[a][c] + distances[b][d];
                if (new_dist < current_dist) {
                    int delta = new_dist - current_dist;
                    if (delta < 0) {
                        for (int k = 0; k < (j - i) / 2; k++) {
                            int temp = best_tour[i + 1 + k];
                            best_tour[i + 1 + k] = best_tour[j - k];
                            best_tour[j - k] = temp;
                        }
                        shortest_dist += delta;
                        improved = true;
                    }
                }
            }
        }
    }

    TourResult result = {best_tour, shortest_dist};
    return result;
}

int* two_opt_reverse(int** distances, const int* initial_tour, int n) {
    int* best_tour = malloc(n * sizeof(int));
    memcpy(best_tour, initial_tour, n * sizeof(int));
    int longest_dist = calculate_tour_length(best_tour, n, distances);
    bool improved = true;

    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1; i++) {
            for (int j = i + 2; j < n; j++) {
                int* temp_tour = malloc(n * sizeof(int));
                memcpy(temp_tour, best_tour, n * sizeof(int));
                for (int k = 0; k < (j - i) / 2; k++) {
                    int temp = temp_tour[i + 1 + k];
                    temp_tour[i + 1 + k] = temp_tour[j - k];
                    temp_tour[j - k] = temp;
                }
                int new_dist = calculate_tour_length(temp_tour, n, distances);
                if (new_dist > longest_dist) {
                    free(best_tour);
                    best_tour = temp_tour;
                    longest_dist = new_dist;
                    improved = true;
                } else {
                    free(temp_tour);
                }
            }
        }
    }
    return best_tour;
}

TourResult two_opt_and_swap(int** distances, const int* initial_tour, int n) {
    int* worsened_tour = two_opt_reverse(distances, initial_tour, n);
    TourResult best = two_opt_swap(distances, worsened_tour, n);
    free(worsened_tour);
    int* best_tour = best.tour;
    int shortest_dist = best.dist;
    int* current_tour = malloc(n * sizeof(int));
    memcpy(current_tour, best_tour, n * sizeof(int));
    bool improved = true;

    while (improved) {
        improved = false;
        for (int i = 0; i < n - 1; i++) {
            for (int j = 0; j < n; j++) {
                if (i != j) {
                    int temp = current_tour[i];
                    current_tour[i] = current_tour[j];
                    current_tour[j] = temp;
                    TourResult temp_result = two_opt_swap(distances, current_tour, n);
                    if (temp_result.dist < shortest_dist) {
                        free(best_tour);
                        best_tour = temp_result.tour;
                        memcpy(current_tour, best_tour, n * sizeof(int));
                        shortest_dist = temp_result.dist;
                        improved = true;
                        printf("2-opt improvement: %d\n", shortest_dist);
                        break;
                    } else {
                        free(temp_result.tour);
                    }
                }
            }
            if (improved) break;
        }
    }
end:
    free(current_tour);
    TourResult result = {best_tour, shortest_dist};
    return result;
}

int* nearest_neighbor(int** distances, int n, int initial_point) {
    int* tour = malloc(n * sizeof(int));
    bool* visited = calloc(n, sizeof(bool));
    int tour_size = 0;

    tour[tour_size++] = 0;
    visited[0] = true;
    if (initial_point != 0 && initial_point >= 0 && initial_point < n) {
        tour[tour_size++] = initial_point;
        visited[initial_point] = true;
    }
    int last_element = initial_point != 0 ? initial_point : 0;

    while (tour_size < n) {
        int min_dist = INT_MAX;
        int next_node = -1;
        for (int j = 0; j < n; j++) {
            if (!visited[j] && distances[last_element][j] < min_dist) {
                min_dist = distances[last_element][j];
                next_node = j;
            }
        }
        if (next_node != -1) {
            tour[tour_size++] = next_node;
            visited[next_node] = true;
            last_element = next_node;
        }
    }

    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", tour[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\n");

    free(visited);
    return tour;
}

void free_distances(int** distances, int num_points) {
    for (int i = 0; i < num_points; i++) free(distances[i]);
    free(distances);
}

TourResult solve_tsp(Point* points, int num_points) { 
    clock_t start = clock();
    int** distances = pre_process(points, num_points);
    int initial_point = 1;
    int limit = initial_point + MAX_RUNS;

    int* best_tour = NULL;
    int shortest_dist = INT_MAX;

    int tourfile_number = create_tour_file();

    while (initial_point < limit) {
        printf("current run: [%d], time: %.2f seconds\n", initial_point, (double)(clock() - start) / CLOCKS_PER_SEC);

        int* initial_tour = nearest_neighbor(distances, num_points, initial_point);
        TourResult result = two_opt_and_swap(distances, initial_tour, num_points);

        free(initial_tour);
        if (result.dist < shortest_dist) {
            if (best_tour) free(best_tour);
            best_tour = result.tour;
            shortest_dist = result.dist;
            printf("New shortest dist: %d\n", shortest_dist);
            printf("Saving tour...\n");
            update_tour_file(best_tour, num_points, shortest_dist, (double)(clock() - start) / CLOCKS_PER_SEC, tourfile_number);
            printf("Saved\n");
        } else {
            free(result.tour);
            update_tour_file(best_tour, num_points, shortest_dist, (double)(clock() - start) / CLOCKS_PER_SEC, tourfile_number);
        }

        initial_point++;
    }


    printf("Total time: %.2f seconds\n", (double)(clock() - start) / CLOCKS_PER_SEC);

    free_distances(distances, num_points);
    TourResult result = {best_tour, shortest_dist};
    return result;
}

int main() {
    int num_points;
    Point* tsp_points = parse_tsp_file(FILEPATH, &num_points);
    if (!tsp_points) {
        printf("Failed to parse TSP file %s\n", FILEPATH);
        return 1;
    }
    int opt_num_points;
    int* opt_tour = parse_tour_file(OPT_FILEPATH, &opt_num_points);
    int opt_dist = 0;
    if (opt_tour != NULL) {
        opt_dist = calculate_tour_distance(tsp_points, opt_tour, num_points);
    }
    printf("Optimal distance: %d\n", opt_dist);

    TourResult result = solve_tsp(tsp_points, num_points);
    int* tour = result.tour;
    int dist = result.dist;

    printf("Optimal distance: %d\n", opt_dist);
    printf("Best found distance: %d\n", dist);

    free(tour);
    free(opt_tour);
    free(tsp_points);
    return 0;
}
