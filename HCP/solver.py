import random
import time

from parse_hcp import (
    parse_hcp
)

from save_file import (
    save_tour_file
)


def create_hamiltonian_graph_guaranteed():
    num_nodes = random.randint(100, 100)
    graph = {i: [] for i in range(num_nodes)}

    nodes = list(range(num_nodes))
    random.shuffle(nodes)
    for i in range(num_nodes):
        u, v = nodes[i], nodes[(i + 1) % num_nodes]
        graph[u].append(v)
        graph[v].append(u)
    hamiltonian_cycle = nodes + [nodes[0]]

    for u in range(num_nodes):
        for v in range(u + 1, num_nodes):
            if v not in graph[u] and random.random() < 0.01:
                graph[u].append(v)
                graph[v].append(u)

    return graph, hamiltonian_cycle


def HC_to_TSP(graph):
    processed_distances = {}

    for idx in graph:
        processed_distances[idx] = {}

        for other_idx in graph:
            if idx != other_idx:
                if other_idx in graph[idx]:
                    processed_distances[idx][other_idx] = 1
                else:
                    processed_distances[idx][other_idx] = 2

    return processed_distances


def calculate_tour_length(tour, processed_distances):
    length = 0
    for i in range(len(tour)):
        current = tour[i]
        next = tour[(i + 1) % len(tour)]
        length += processed_distances[current][next]
    return length


def two_opt_swap(processed_distances, initial_tour):
    best_tour = initial_tour.copy()
    n = len(best_tour)
    shortest_dist = calculate_tour_length(best_tour, processed_distances)
    improved = True

    while improved:
        improved = False
        for i in range(n - 1):
            for j in range(i + 2, n):
                a, b = best_tour[i], best_tour[(i + 1) % n]
                c, d = best_tour[j], best_tour[(j + 1) % n]

                current_dist = processed_distances[a][b] + processed_distances[c][d]
                new_dist = processed_distances[a][c] + processed_distances[b][d]

                if new_dist < current_dist:
                    delta = new_dist - current_dist
                    if delta < 0:
                        best_tour[i + 1:j + 1] = best_tour[j:i:-1]
                        shortest_dist += delta
                        improved = True

                        if shortest_dist == len(processed_distances):
                            return best_tour, shortest_dist

                        break
            if improved:
                break

    return best_tour, shortest_dist


def two_opt_reverse(processed_distances, initial_tour):
    best_tour = initial_tour.copy()
    n = len(best_tour)
    longest_dist = calculate_tour_length(initial_tour, processed_distances)
    improved = True

    while improved:
        improved = False

        for i in range(n - 1):
            for j in range(i + 2, n):
                temp_tour = best_tour.copy()
                temp_tour[i+1:j+1] = temp_tour[j:i:-1]
                new_dist = calculate_tour_length(temp_tour, processed_distances)

                if new_dist > longest_dist:
                    best_tour = temp_tour
                    longest_dist = new_dist
                    improved = True
                    break
            if improved:
                break

    return best_tour


def two_opt_and_swap(processed_distances, initial_tour):
    worsened_tour = two_opt_reverse(processed_distances, initial_tour)
    best_tour, shortest_dist = two_opt_swap(processed_distances, worsened_tour)
    current_tour = best_tour.copy()
    n = len(best_tour)
    global_improved = True

    while global_improved:
        global_improved = False

        for i in range(n - 1):
            for j in range(n):
                if i != j:
                    current_tour[i], current_tour[j] = current_tour[j], current_tour[i]

                    tour_2opt, dist_2opt = two_opt_swap(processed_distances, current_tour)
                    if dist_2opt < shortest_dist:
                        best_tour = tour_2opt
                        current_tour = best_tour.copy()
                        shortest_dist = dist_2opt
                        global_improved = True

                        print(f"2-opt improvement: {shortest_dist}")

                        if shortest_dist == len(processed_distances):
                            return best_tour, shortest_dist

    return best_tour, shortest_dist


def nearest_neighbor(processed_distances, initial_point):
    current_tour = {1, initial_point}
    last_element = initial_point

    while len(current_tour) < len(processed_distances):
        added = False

        for point in processed_distances[last_element]:
            if point not in current_tour and processed_distances[last_element][point] == 1:
                current_tour.add(point)
                last_element = point
                added = True
                break

        if added is False:
            for point in processed_distances[last_element]:
                if point not in current_tour:
                    current_tour.add(point)
                    last_element = point
                    break

    print(current_tour)
    return list(current_tour)


def is_graph_valid(graph):
    for node, neighbors in graph.items():
        if not neighbors:
            print(f"Error: node {node} is isolated.")
            return False

    for node, neighbors in graph.items():
        for neighbor in neighbors:
            if node not in graph.get(neighbor, []):
                print(f"Error: connection between {node} and {neighbor} isn't valid")
                return False

    print("Graph looks valid.")
    return True


def main():
    graph = parse_hcp("HCP_instances/undefined.hcp")

    is_valid = is_graph_valid(graph)

    if is_valid is False:
        return

    print(len(graph))
    print(graph)

    processed_distances = HC_to_TSP(graph)

    best_tour, shortest_dist = None, float("inf")

    counter = 2
    start = time.time()
    while counter < len(graph):
        print(f"Runs: {counter}, time: {round(time.time() - start, 2)}")
        initial_tour = nearest_neighbor(processed_distances, counter)
        counter += 1

        tour, dist = two_opt_and_swap(processed_distances, initial_tour)
        if dist < shortest_dist:
            shortest_dist = dist
            best_tour = tour

            if shortest_dist == len(processed_distances):
                break

    print(f"best dist: {shortest_dist}")
    print(f"min dist: {len(tour)}")
    print(best_tour)
    print(f"Total time {round(time.time() - start, 2)} seconds")
    print("-------")

    save_tour_file(tour)


if __name__ == "__main__":
    main()
