import time

# Global variables
FILEPATH = "HCP_instances/undefined.hcp"
NAME = "undefined"


def parse_hcp(file_content):
    with open(file_content, 'r') as file:
        file_content = file.read()
        file.close()

    graph = {}
    edge_data_section = False

    for line in file_content.splitlines():
        line = line.strip()

        if line.startswith("EDGE_DATA_SECTION"):
            edge_data_section = True
            continue

        if line == "-1":
            break

        if edge_data_section:
            parts = line.split()
            if len(parts) == 2:
                u = int(parts[0])
                v = int(parts[1])

                if u not in graph:
                    graph[u] = []
                if v not in graph:
                    graph[v] = []

                if v not in graph[u]:
                    graph[u].append(v)
                if u not in graph[v]:
                    graph[v].append(u)

    return graph


# Save a solution in TSBLIB format.
def save_tour_file(tour, time):
    filepath = f"HCP_results/{NAME}.tour"
    num_nodes = len(tour)

    with open(filepath, "w") as f:
        f.write(f"NAME : {NAME}\n")
        f.write("TYPE : HCP TOUR\n")
        f.write(f"COMMENT : {num_nodes}-node graph, total time {time} seconds\n")
        f.write(f"DIMENSION : {num_nodes}\n")
        f.write("EDGE_DATA_FORMAT : EDGE_LIST\n")
        f.write("EDGE_DATA_SECTION\n")

        for node in tour:
            f.write(f"{node}\n")

        f.write("-1\n")
        f.write("EOF\n")
        f.close()

    print(f"Result saved in {filepath}")


def generate_distance_matrix(graph):
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


def validate_graph(graph):
    for node in graph:
        if len(graph[node]) == 0:
            print(f"Error: node {node} is isolated.")
            return False

        if node in graph[node]:
            print(f"Error: node {node} have himself as neighbor.")
            return False

        for neighbor in graph[node]:
            if node not in graph[neighbor]:
                print(f"Error: connection between {node} and {neighbor} is not bidirectional")
                return False

    print("Graph looks valid.")
    return True


def solve_HCP(graph):
    start = time.time()
    processed_distances = generate_distance_matrix(graph)
    best_tour, shortest_dist = None, float("inf")
    initial_point = 2

    while initial_point < len(graph):
        print(f"Runs: {initial_point - 1}, time: {round(time.time() - start, 2)}")

        initial_tour = nearest_neighbor(processed_distances, initial_point)
        tour, dist = two_opt_and_swap(processed_distances, initial_tour)

        if dist < shortest_dist:
            shortest_dist = dist
            best_tour = tour

            if shortest_dist == len(processed_distances):
                break

        initial_point += 1

    print(best_tour)

    print("Saving result...")
    save_tour_file(tour, round(time.time() - start, 2))
    print("Saved.")

    print(f"Total time {round(time.time() - start, 2)} seconds")

    return best_tour, shortest_dist


def main():
    graph = parse_hcp(FILEPATH)

    if not validate_graph(graph):
        return

    best_tour, shortest_dist = solve_HCP(graph)

    print(f"best dist: {shortest_dist}")
    print(f"min dist: {len(graph)}")


if __name__ == "__main__":
    main()
