import os


def create_tour_file(tour, name, dist, time):
    counter = 1

    while True:
        filename = f"TSP_results/{name}_{counter}.tour"
        if not os.path.exists(filename):
            with open(filename, "w") as file:
                file.write(f"NAME: {name}_{counter}.tour\n")
                file.write(f"COMMENT: Tour length {dist}, time {time}\n")
                file.write("TYPE: TOUR\n")
                file.write(f"DIMENSION: {len(tour)}\n")
                file.write("TOUR_SECTION\n")

                for point in tour:
                    file.write(f"{point+1}\n")
                file.write("-1\n")
                file.write("EOF\n")
            break
        else:
            counter += 1

    return filename


def save_created_tour_file(tour, filename, dist, time):
    with open(filename, "w") as file:
        file.write(f"NAME: {filename}\n")
        file.write(f"COMMENT: Tour length {dist}, time {time} seconds\n")
        file.write("TYPE: TOUR\n")
        file.write(f"DIMENSION: {len(tour)}\n")
        file.write("TOUR_SECTION\n")

        for point in tour:
            file.write(f"{point+1}\n")
        file.write("-1\n")
        file.write("EOF\n")
