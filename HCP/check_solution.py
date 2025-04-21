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

        if line == "EDGE_DATA_SECTION":
            tour_section = True
            continue
        if line == "-1":
            break

        if tour_section:
            parts = line.split()
            if len(parts) == 1:
                point = int(parts[0])
                coordinates_points.append(point)

    return coordinates_points


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


def check_solution(graph, tour):
    n = len(tour)

    for i in range(n - 1):
        node = tour[i]
        next_node = tour[i + 1]

        if next_node not in graph[node]:
            print("Solution is invalid!")
            print(node, next_node)
            return False

    print("Solution looks valid")


def main():
    tour = parse_tour_file("HCP_results/150_hard.tour")
    graph = parse_hcp("HCP_instances/150_hard.hcp")

    check_solution(graph, tour)


if __name__ == "__main__":
    main()
