def save_sudoku_file(tour, subgraphs, filename):

    if not filename.endswith('.txt'):
        filename += '_solution.txt'
    else:
        splited = filename.split(".")
        splited[0] += "_solution"
        filename = ".".join(splited)

    filename = f"sudoku_results/{filename}"

    rows = subgraphs[0]

    with open(filename, 'w') as f:
        for row in rows:
            for cell in row:
                f.write(f"{tour[cell]} ")
            f.write("\n")
