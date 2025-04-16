#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_NODES 1000
#define MAX_NEIGHBORS 1000

typedef struct {
    int* neighbors;
    int num_neighbors;
} Node;

typedef struct {
    int* tour;
    int dist;
} TourResult;

Node* parse_hcp(const char* filename, int* num_nodes) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Error: Unable to open file %s\n", filename);
        return NULL;
    }

    Node* graph = calloc(MAX_NODES, sizeof(Node));
    if (!graph) {
        fclose(file);
        return NULL;
    }

    char line[256];
    bool edge_section = false;
    int max_node = -1;

    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "EDGE_DATA_SECTION")) {
            edge_section = true;
            continue;
        }
        if (strcmp(line, "-1\n") == 0) break;

        if (edge_section) {
            int u, v;
            if (sscanf(line, "%d %d", &u, &v) == 2) {
                u--;
                v--;
                if (u >= MAX_NODES || v >= MAX_NODES || u < 0 || v < 0) {
                    printf("Error: Node index out of bounds: %d or %d\n", u + 1, v + 1);
                    fclose(file);
                    free(graph);
                    return NULL;
                }
                if (!graph[u].neighbors) {
                    graph[u].neighbors = malloc(MAX_NEIGHBORS * sizeof(int));
                    if (!graph[u].neighbors) {
                        fclose(file);
                        free(graph);
                        return NULL;
                    }
                    graph[u].num_neighbors = 0;
                }
                if (!graph[v].neighbors) {
                    graph[v].neighbors = malloc(MAX_NEIGHBORS * sizeof(int));
                    if (!graph[v].neighbors) {
                        fclose(file);
                        free(graph);
                        return NULL;
                    }
                    graph[v].num_neighbors = 0;
                }
                if (graph[u].num_neighbors < MAX_NEIGHBORS && graph[v].num_neighbors < MAX_NEIGHBORS) {
                    graph[u].neighbors[graph[u].num_neighbors++] = v;
                    graph[v].neighbors[graph[v].num_neighbors++] = u;
                    if (u > max_node) max_node = u;
                    if (v > max_node) max_node = v;
                }
            }
        }
    }
    *num_nodes = max_node + 1;
    fclose(file);
    return graph;
}

void save_tour_file(const int* tour, int num_nodes, const char* filename, const char* name) {
    char filepath[256];
    snprintf(filepath, sizeof(filepath), "HCP_results/%s", filename[0] == '\0' ? "undefined.tour" : filename);
    if (!strstr(filepath, ".tour")) strcat(filepath, ".tour");

    FILE* f = fopen(filepath, "w");
    if (!f) {
        printf("Error: Unable to create file %s\n", filepath);
        return;
    }

    fprintf(f, "NAME : %s\n", name[0] == '\0' ? "Undefined" : name);
    fprintf(f, "TYPE : HCP TOUR\n");
    fprintf(f, "COMMENT : %d-node graph\n", num_nodes);
    fprintf(f, "DIMENSION : %d\n", num_nodes);
    fprintf(f, "EDGE_DATA_FORMAT : EDGE_LIST\n");
    fprintf(f, "EDGE_DATA_SECTION\n");
    for (int i = 0; i < num_nodes; i++) fprintf(f, "%d\n", tour[i] + 1);
    fprintf(f, "-1\nEOF\n");
    fclose(f);
    printf("Tour saved to %s\n", filepath);
}

Node* create_hamiltonian_graph_guaranteed(int* num_nodes, int** hamiltonian_cycle) {
    *num_nodes = 100;
    Node* graph = calloc(*num_nodes, sizeof(Node));
    int* nodes = malloc(*num_nodes * sizeof(int));
    for (int i = 0; i < *num_nodes; i++) {
        nodes[i] = i;
        graph[i].neighbors = malloc(MAX_NEIGHBORS * sizeof(int));
        graph[i].num_neighbors = 0;
    }

    srand(time(NULL));
    for (int i = *num_nodes - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = nodes[i];
        nodes[i] = nodes[j];
        nodes[j] = temp;
    }

    for (int i = 0; i < *num_nodes; i++) {
        int u = nodes[i], v = nodes[(i + 1) % *num_nodes];
        graph[u].neighbors[graph[u].num_neighbors++] = v;
        graph[v].neighbors[graph[v].num_neighbors++] = u;
    }

    *hamiltonian_cycle = malloc((*num_nodes + 1) * sizeof(int));
    memcpy(*hamiltonian_cycle, nodes, *num_nodes * sizeof(int));
    (*hamiltonian_cycle)[*num_nodes] = nodes[0];

    for (int u = 0; u < *num_nodes; u++) {
        for (int v = u + 1; v < *num_nodes; v++) {
            bool exists = false;
            for (int k = 0; k < graph[u].num_neighbors; k++) {
                if (graph[u].neighbors[k] == v) {
                    exists = true;
                    break;
                }
            }
            if (!exists && (rand() % 10000) < 100) {
                if (graph[u].num_neighbors < MAX_NEIGHBORS && graph[v].num_neighbors < MAX_NEIGHBORS) {
                    graph[u].neighbors[graph[u].num_neighbors++] = v;
                    graph[v].neighbors[graph[v].num_neighbors++] = u;
                }
            }
        }
    }
    free(nodes);
    return graph;
}

int** HC_to_TSP(Node* graph, int num_nodes) {
    int** distances = malloc(num_nodes * sizeof(int*));
    for (int i = 0; i < num_nodes; i++) {
        distances[i] = malloc(num_nodes * sizeof(int));
    }

    for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < num_nodes; j++) {
            if (i == j) {
                distances[i][j] = 0;
                continue;
            }
            bool adjacent = false;
            for (int k = 0; k < graph[i].num_neighbors; k++) {
                if (graph[i].neighbors[k] == j) {
                    adjacent = true;
                    break;
                }
            }
            distances[i][j] = adjacent ? 1 : 2;
        }
    }
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
                        if (shortest_dist == n) goto end;
                        break;
                    }
                }
            }
            if (improved) break;
        }
    }
end:
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
                    break;
                } else {
                    free(temp_tour);
                }
            }
            if (improved) break;
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
    bool global_improved = true;

    while (global_improved) {
        global_improved = false;
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
                        global_improved = true;
                        printf("2-opt improvement: %d\n", shortest_dist);
                        if (shortest_dist == n) goto end;
                    } else {
                        free(temp_result.tour);
                        temp = current_tour[i];
                        current_tour[i] = current_tour[j];
                        current_tour[j] = temp;
                    }
                }
            }
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

    tour[tour_size++] = 1;
    visited[1] = true;
    if (initial_point != 1 && initial_point >= 0 && initial_point < n) {
        tour[tour_size++] = initial_point;
        visited[initial_point] = true;
    }
    int last_element = initial_point != 1 ? initial_point : 1;

    while (tour_size < n) {
        bool added = false;
        for (int j = 0; j < n; j++) {
            if (!visited[j] && distances[last_element][j] == 1) {
                tour[tour_size++] = j;
                visited[j] = true;
                last_element = j;
                added = true;
                break;
            }
        }
        if (!added) {
            for (int j = 0; j < n; j++) {
                if (!visited[j]) {
                    tour[tour_size++] = j;
                    visited[j] = true;
                    last_element = j;
                    break;
                }
            }
        }
    }

    // Print tour
    printf("[");
    for (int i = 0; i < n; i++) {
        printf("%d", tour[i]);
        if (i < n - 1) printf(", ");
    }
    printf("]\n");

    free(visited);
    return tour;
}

bool is_graph_valid(Node* graph, int num_nodes) {
    for (int i = 0; i < num_nodes; i++) {
        if (graph[i].num_neighbors == 0) {
            printf("Error: node %d is isolated.\n", i);
            return false;
        }
        for (int j = 0; j < graph[i].num_neighbors; j++) {
            int neighbor = graph[i].neighbors[j];
            bool found = false;
            for (int k = 0; k < graph[neighbor].num_neighbors; k++) {
                if (graph[neighbor].neighbors[k] == i) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                printf("Error: connection between %d and %d isn't valid\n", i, neighbor);
                return false;
            }
        }
    }
    printf("Graph looks valid.\n");
    return true;
}

void free_graph(Node* graph, int num_nodes) {
    for (int i = 0; i < num_nodes; i++) {
        if (graph[i].neighbors) free(graph[i].neighbors);
    }
    free(graph);
}

void free_distances(int** distances, int num_nodes) {
    for (int i = 0; i < num_nodes; i++) free(distances[i]);
    free(distances);
}

void print_graph(Node* graph, int num_nodes) {
    printf("{");
    for (int i = 0; i < num_nodes; i++) {
        printf("%d: [", i);
        for (int j = 0; j < graph[i].num_neighbors; j++) {
            printf("%d", graph[i].neighbors[j]);
            if (j < graph[i].num_neighbors - 1) printf(", ");
        }
        printf("]");
        if (i < num_nodes - 1) printf(", ");
    }
    printf("}\n");
}

int main() {
    srand(time(NULL));
    int num_nodes;
    Node* graph = parse_hcp("HCP_instances/150_hard.hcp", &num_nodes);
    if (!graph) {
        printf("Failed to parse HCP file.\n");
        return 1;
    }

    if (!is_graph_valid(graph, num_nodes)) {
        free_graph(graph, num_nodes);
        return 1;
    }

    printf("%d\n", num_nodes);
    print_graph(graph, num_nodes);

    int** distances = HC_to_TSP(graph, num_nodes);

    int* best_tour = NULL;
    int shortest_dist = INT_MAX;
    int counter = 2;
    clock_t start = clock();

    while (counter < num_nodes) {
        double time_elapsed = (double)(clock() - start) / CLOCKS_PER_SEC;
        printf("Runs: %d, time: %.2f\n", counter, time_elapsed);
        int* initial_tour = nearest_neighbor(distances, num_nodes, counter);
        counter++;

        TourResult result = two_opt_and_swap(distances, initial_tour, num_nodes);
        free(initial_tour);
        if (result.dist < shortest_dist) {
            if (best_tour) free(best_tour);
            best_tour = result.tour;
            shortest_dist = result.dist;
            if (shortest_dist == num_nodes) break;
        } else {
            free(result.tour);
        }
    }

    printf("best dist: %d\n", shortest_dist);
    printf("min dist: %d\n", num_nodes);
    printf("[");
    for (int i = 0; i < num_nodes; i++) {
        printf("%d", best_tour[i]);
        if (i < num_nodes - 1) printf(", ");
    }
    printf("]\n");
    printf("Total time: %.2f seconds\n", (double)(clock() - start) / CLOCKS_PER_SEC);
    printf("-------\n");

    save_tour_file(best_tour, num_nodes, "150_hard", "150_hard");
    free(best_tour);
    free_distances(distances, num_nodes);
    free_graph(graph, num_nodes);
    return 0;
}
