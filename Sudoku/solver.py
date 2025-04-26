import time
import math

# Global variables
FILEPATH = "Sudoku_instances/april_5_2025.txt"
NAME = "april_5_2025"
MAX_RUNS = 250


def parse_sudoku_file(filepath):
    rows = []

    with open(filepath, 'r') as file:
        lines = file.readlines()
        file.close()

    for line in lines:
        numbers = line.split()
        row = []

        if len(numbers) == 0:
            break

        for number in numbers:
            row.append(int(number))
        rows.append(row)

    return rows


def save_sudoku_file(tour, subgraphs, time):
    filepath = f"Sudoku_results/{NAME}_solution.txt"
    rows = subgraphs[0]

    with open(filepath, 'w') as f:
        for row in rows:
            for cell in row:
                f.write(f"{tour[cell]} ")
            f.write("\n")

        f.write(f"\ntotal time {time} seconds")
        f.close()

    print(f"Result saved in {filepath}")


def generate_cell_matrix(sudoku):
    cell_matrix = {}
    n = len(sudoku[0])

    for i in range(n):
        for j in range(n):
            cell_matrix[(i, j)] = sudoku[i][j]

    return cell_matrix


def generate_subgraphs(numbers):
    n = len(numbers)
    sqrt = int(math.sqrt(n))

    rows = []
    columns = []
    blocks = []

    # Rows
    for i in range(n):
        row = set()

        for j in range(n):
            row.add((i, j))

        rows.append(row)

    # Columns
    for i in range(n):
        column = set()

        for j in range(n):
            column.add((j, i))

        columns.append(column)

    # Blocks
    for i in range(0, n, sqrt):
        for j in range(0, n, sqrt):
            block = set()

            for i2 in range(i, i + sqrt):
                for j2 in range(j, j + sqrt):
                    block.add((i2, j2))

            blocks.append(block)

    return [rows, columns, blocks]


def calculate_global_cost(tour, subgraphs):
    global_cost = 0
    n = len(subgraphs[0])

    # Rows
    for row in subgraphs[0]:
        unique_elements = set()

        for cell in row:
            unique_elements.add(tour[cell])

        global_cost += len(unique_elements) + (n - len(unique_elements)) * 2

    # Columns
    for column in subgraphs[1]:
        unique_elements = set()

        for cell in column:
            unique_elements.add(tour[cell])

        global_cost += len(unique_elements) + (n - len(unique_elements)) * 2

    # Blocks
    for block in subgraphs[2]:
        unique_elements = set()

        for cell in block:
            unique_elements.add(tour[cell])

        global_cost += len(unique_elements) + (n - len(unique_elements)) * 2

    return global_cost


def generate_greedy_tour(cell_matrix, numbers, specific_cell, specific_number, subgraphs):
    tour = {}

    for cell in cell_matrix:
        tour[cell] = 0

    for row in subgraphs[0]:
        available_numbers = set(numbers)

        for cell in row:
            if cell_matrix[cell] != 0 and cell_matrix[cell] in available_numbers:
                tour[cell] = cell_matrix[cell]
                available_numbers.remove(cell_matrix[cell])

        if specific_cell in row and specific_number in available_numbers:
            tour[specific_cell] = specific_number
            available_numbers.remove(specific_number)

        for cell in row:
            if tour[cell] == 0:
                for number in available_numbers:
                    tour[cell] = number
                    break

                available_numbers.remove(number)

    print(tour)

    return tour


# Adapted for Sudoku
def two_opt(tour, numbers, subgraphs, cell_matrix, cells):
    global_cost = calculate_global_cost(tour, subgraphs)
    improved = True

    n = len(cells)

    while improved:
        improved = False

        for i in range(n - 1):
            for j in range(i + 1, n):
                cell1 = cells[i]
                cell2 = cells[j]

                if cell_matrix[cell1] == 0 and cell_matrix[cell2] == 0:
                    if tour[cell1] != tour[cell2]:
                        temp = tour[cell1]
                        tour[cell1] = tour[cell2]
                        tour[cell2] = temp

                        new_global_cost = calculate_global_cost(tour, subgraphs)

                        if new_global_cost < global_cost:
                            global_cost = new_global_cost
                            improved = True
                        else:
                            tour[cell2] = tour[cell1]
                            tour[cell1] = temp

    return tour, global_cost


# Adapted for Sudoku
def two_opt_reverse(tour, numbers, subgraphs, cell_matrix, cells):
    global_cost = calculate_global_cost(tour, subgraphs)
    improved = True

    n = len(cells)

    while improved:
        improved = False

        for i in range(n - 1):
            for j in range(i + 1, n):
                cell1 = cells[i]
                cell2 = cells[j]

                if cell_matrix[cell1] == 0 and cell_matrix[cell2] == 0:
                    if tour[cell1] != tour[cell2]:
                        temp = tour[cell1]
                        tour[cell1] = tour[cell2]
                        tour[cell2] = temp

                        new_global_cost = calculate_global_cost(tour, subgraphs)

                        if new_global_cost > global_cost:
                            global_cost = new_global_cost
                            improved = True
                        else:
                            tour[cell2] = tour[cell1]
                            tour[cell1] = temp

    return tour


def two_opt_and_swap(initial_tour, numbers, subgraphs, cell_matrix):
    cells = []

    for row in subgraphs[0]:
        for cell in row:
            cells.append(cell)

    worsened_tour = two_opt_reverse(initial_tour, numbers, subgraphs, cell_matrix, cells)
    best_tour, global_cost = two_opt(worsened_tour, numbers, subgraphs, cell_matrix, cells)
    current_tour = best_tour.copy()
    min_cost = len(numbers) ** 2 * 3
    n = len(cells)

    improved = True

    if global_cost == min_cost:
        print("Found!")
        return best_tour, min_cost

    while improved:
        improved = False

        for i in range(n - 1):
            for j in range(n):
                if i != j:
                    cell1 = cells[i]
                    cell2 = cells[j]

                    if cell_matrix[cell1] == 0 and cell_matrix[cell2] == 0:
                        if current_tour[cell1] != current_tour[cell2]:
                            temp = current_tour[cell1]
                            current_tour[cell1] = current_tour[cell2]
                            current_tour[cell2] = temp
                            current_tour, new_global_cost = two_opt(current_tour, numbers, subgraphs, cell_matrix, cells)

                            if new_global_cost < global_cost:
                                best_tour = current_tour
                                global_cost = new_global_cost
                                improved = True
                                print(global_cost)

                                if global_cost == min_cost:
                                    print("Found!")
                                    return best_tour, global_cost
                                break
            if improved:
                break

    print(best_tour)
    return best_tour, global_cost


def validate_sudoku(sudoku, subgraphs):
    for structs in subgraphs:
        for struct in structs:
            used_numbers = {}

            for cell in struct:
                i = cell[0]
                j = cell[-1]

                if sudoku[i][j] != 0:
                    if sudoku[i][j] not in used_numbers:
                        used_numbers[sudoku[i][j]] = 1
                    else:
                        used_numbers[sudoku[i][j]] += 1

            print(used_numbers)

            for number in used_numbers:
                if used_numbers[number] > 1:
                    print("Invalid sudoku")
                    return False

    print("Sudoku seems valid.")
    return True


def solve_sudoku(sudoku, numbers, subgraphs):
    cell_matrix = generate_cell_matrix(sudoku)

    start = time.time()

    counter = 0
    run_counter = 0
    cell_variations = []

    min_cost = len(numbers) ** 2 * 3
    lowest_cost = float("inf")

    for cell in cell_matrix:
        if cell_matrix[cell] == 0:
            for number in numbers:
                cell_variations.append([cell, number])

                counter += 1
                if counter >= MAX_RUNS:
                    break
            if counter >= MAX_RUNS:
                break

    for cell_number in cell_variations:
        run_counter += 1
        print(f"Current run: {run_counter}")
        print(f"{round(time.time() - start, 2)} seconds")
        print(cell_number[0], cell_number[1])

        initial_tour = generate_greedy_tour(cell_matrix, numbers, cell_number[0], cell_number[1], subgraphs)
        tour, cost = two_opt_and_swap(initial_tour, numbers, subgraphs, cell_matrix)

        if cost < lowest_cost:
            lowest_cost = cost

        if cost == min_cost:
            break

    print("Saving result...")
    save_sudoku_file(tour, subgraphs, round(time.time() - start, 2))
    print("Saved.")

    print(f"Total time: {round(time.time() - start, 2)} seconds")
    print(f"Lowest cost: {lowest_cost}")


def main():
    sudoku = parse_sudoku_file(FILEPATH)
    numbers = list(range(1, len(sudoku) + 1))
    subgraphs = generate_subgraphs(numbers)

    is_valid = validate_sudoku(sudoku, subgraphs)
    if is_valid is False:
        return

    solve_sudoku(sudoku, numbers, subgraphs)


if __name__ == "__main__":
    main()
