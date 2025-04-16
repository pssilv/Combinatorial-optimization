def parse_sudoku_file(file_path):
    rows = []

    with open(file_path, 'r') as file:
        lines = file.readlines()

    for line in lines:
        numbers = line.split()
        row = []

        if len(numbers) == 0:
            break

        for number in numbers:
            row.append(int(number))
        rows.append(row)

    return rows
