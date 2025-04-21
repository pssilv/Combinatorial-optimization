import math
import time
import os

#  Global variables
MAX_RUNS = 5
NAME = "pr76"
FILEPATH = "TSP_instances/pr76.tsp"
OPT_FILEPATH = "TSP_instances/pr76.opt.tour" #  Optional set to None if theres none.


def parse_tsp_file(file_path):
    coordinates = []

    with open(file_path, 'r') as file:
        lines = file.readlines()
        file.close()

    node_coord_section = False

    for line in lines:
        line = line.strip()

        if line == "NODE_COORD_SECTION":
            node_coord_section = True
            continue
        if line == "EOF":
            break

        if node_coord_section:
            parts = line.split()
            if len(parts) >= 3:
                x, y = round(float(parts[1])), round(float(parts[2]))
                coordinates.append((x, y))

    return coordinates


def parse_tour_file(file_path):
    if file_path is None:
        return None

    coordinates_points = []

    with open(file_path, 'r') as file:
        lines = file.readlines()
        file.close()

    tour_section = False

    for line in lines:
        line = line.strip()

        if line == "TOUR_SECTION":
            tour_section = True
            continue
        if line == "-1":
            break

        if tour_section:
            parts = line.split()
            if len(parts) == 1:
                point = int(parts[0]) - 1
                coordinates_points.append(point)

    return coordinates_points


def create_tour_file():
    counter = 1

    while True:
        filepath = f"TSP_results/{NAME}_{counter}.tour"
        if not os.path.exists(filepath):
            with open(filepath, "w") as file:
                file.close()
            break
        else:
            counter += 1

    print(f"Tour file created in {filepath}")

    return counter


def update_tour_file(tour, dist, time, tourfile_number):
    filepath = f"TSP_results/{NAME}_{tourfile_number}.tour"

    with open(filepath, "w") as file:
        file.write(f"NAME: {NAME}\n")
        file.write(f"COMMENT: Tour length {dist}, total time {time} seconds\n")
        file.write("TYPE: TOUR\n")
        file.write(f"DIMENSION: {len(tour)}\n")
        file.write("TOUR_SECTION\n")

        for point in tour:
            file.write(f"{point+1}\n")
        file.write("-1\n")
        file.write("EOF\n")
        file.close()

    print(f"Tour updated in {filepath}")


def calculate_distance(p1, p2):
    return round(math.sqrt((p1[0] - p2[0])**2 + (p1[1] - p2[1])**2))


def calculate_tour_length(tour, processed_distances):
    n = len(tour)
    length = 0
    for i in range(n):
        current = tour[i]
        next = tour[(i + 1) % n]
        length += processed_distances[current][next]

    return length


def calculate_tour_distance(points, tour):
    total_distance = 0

    for idx in range(len(tour) - 1):
        point1 = tour[idx]
        point2 = tour[idx + 1]

        initial_point = points[point1]
        target_point = points[point2]

        distance = calculate_distance(initial_point, target_point)
        total_distance += distance

    total_distance += calculate_distance(points[tour[-1]], points[tour[0]])

    return total_distance


def pre_process(points_position):
    processed_distances = {}

    for idx in range(len(points_position)):
        processed_distances[idx] = {}
        distances = {}

        for other_idx in range(len(points_position)):
            if idx != other_idx:
                distance = calculate_distance(points_position[idx], points_position[other_idx])
                distances[other_idx] = round(distance)

        for kv in sorted(distances.items(), key=lambda kv: (kv[1], kv[0])):
            processed_distances[idx][kv[0]] = kv[1]

    return processed_distances


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
    improved = True

    while improved:
        improved = False

        for i in range(n - 1):
            for j in range(n):
                if i != j:
                    current_tour[i], current_tour[j] = current_tour[j], current_tour[i]
                    new_tour, new_dist = two_opt_swap(processed_distances, current_tour)
                    if new_dist < shortest_dist:
                        best_tour = new_tour
                        current_tour = best_tour.copy()
                        shortest_dist = new_dist
                        improved = True
                        print(f"2-opt improvement: {shortest_dist}")

    return best_tour, shortest_dist


def nearest_neighbor(processed_distances, initial_point):
    current_tour = {0, initial_point}
    last_element = initial_point

    while len(current_tour) < len(processed_distances):
        for point in processed_distances[last_element]:
            if point not in current_tour:
                current_tour.add(point)
                last_element = point
                break

    print(current_tour)
    return list(current_tour)


def solve_tsp(points):
    start = time.time()
    processed_distances = pre_process(points)
    initial_point = 1
    limit = initial_point + MAX_RUNS

    best_tour = None
    shortest_dist = float("inf")

    tourfile_number = create_tour_file()

    while initial_point < limit:
        print(f"current run: [{initial_point}], time: {round(time.time() - start, 2)} seconds")
        initial_point += 1

        initial_tour = nearest_neighbor(processed_distances, initial_point)
        tour, dist = two_opt_and_swap(processed_distances, initial_tour)

        if dist < shortest_dist:
            shortest_dist = dist
            best_tour = tour

            print(f"New shortest dist: {shortest_dist}")
            print("Saving tour...")
            update_tour_file(best_tour, shortest_dist, round(time.time() - start, 2), tourfile_number)
            print("Saved")
        else:
            update_tour_file(best_tour, shortest_dist, round(time.time() - start, 2), tourfile_number)

    print(f"Total time {round(time.time() - start, 2)} seconds")

    return best_tour, shortest_dist


def main():
    points = parse_tsp_file(FILEPATH)
    opt_tour = parse_tour_file(OPT_FILEPATH)
    opt_dist = 0
    if opt_tour is not None:
        opt_dist = calculate_tour_distance(points, opt_tour)
    print(f"Optimal distance: {opt_dist}")

    tour, dist = solve_tsp(points)

    print(f"Optimal distance: {opt_dist}")
    print(f"Best found distance: {dist}")


if __name__ == "__main__":
    main()
