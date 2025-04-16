import math
import time

from save_file import (
    create_tour_file,
    save_created_tour_file
)


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


def calculate_tour_distance(points, tour_set):
    total_distance = 0

    for idx in range(len(tour_set) - 1):
        initial_point = points[tour_set[idx]]
        target_point = points[tour_set[idx + 1]]

        distance = calculate_distance(initial_point, target_point)
        total_distance += distance

    total_distance += calculate_distance(points[tour_set[-1]], points[tour_set[0]])

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

    return best_tour


def two_opt_and_swap(processed_distances, initial_tour):
    worsened_tour = two_opt_reverse(processed_distances, initial_tour)
    best_tour, shortest_dist = two_opt_swap(processed_distances, worsened_tour)
    n = len(best_tour)
    global_improved = True

    while global_improved:
        global_improved = False

        for i in range(n - 1):
            current_tour = best_tour.copy()
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


def solve_tsp(points_position, instance_name="Undefined", runs=10):
    start = time.time()
    processed_distances = pre_process(points_position)
    initial_point = 1

    best_tour, shortest_dist = list(range(len(processed_distances))), float("inf")
    filename = create_tour_file(best_tour, instance_name, shortest_dist, None)

    while initial_point <= runs:
        initial_tour = nearest_neighbor(processed_distances, initial_point)
        initial_point += 1

        tour, dist = two_opt_and_swap(processed_distances, initial_tour)
        print(f"current run: [{initial_point}], time: {round(time.time() - start, 2)} seconds")

        if dist < shortest_dist:
            shortest_dist = dist
            best_tour = tour

            print(f"New shortest dist: {shortest_dist}")

        print("Saving tour...")
        save_created_tour_file(initial_tour, filename, shortest_dist, round(time.time() - start, 2))
        print("Saved")

    return best_tour, shortest_dist
