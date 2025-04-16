def parse_hcp(file_content):
    with open(file_content, 'r') as file:
        file_content = file.read()

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
