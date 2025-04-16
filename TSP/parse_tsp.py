def parse_tsp_file(file_path):
    coordinates = []

    with open(file_path, 'r') as file:
        lines = file.readlines()

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
    coordinates_points = []

    with open(file_path, 'r') as file:
        lines = file.readlines()

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
