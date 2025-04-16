import os


# Save an Hamiltonian cycle problem in TSPLIB format.
def save_hcp_file(graph_dict, filename="undefined", name="Undefined"):
    output_dir = "HCP_instances"

    if not filename.endswith(".hcp"):
        filename += ".hcp"

    filepath = os.path.join(output_dir, filename)

    num_nodes = len(graph_dict)

    with open(filepath, "w") as f:
        f.write(f"NAME : {name}\n")
        f.write("TYPE : HCP\n")
        f.write(f"COMMENT : {num_nodes}-node graph\n")
        f.write(f"DIMENSION : {num_nodes}\n")
        f.write("EDGE_DATA_FORMAT : EDGE_LIST\n")
        f.write("EDGE_DATA_SECTION\n")

        written_edges = set()
        for node, neighbors in graph_dict.items():
            for neighbor in neighbors:
                node_1 = node + 1
                neighbor_1 = neighbor + 1
                if (node, neighbor) not in written_edges and (neighbor, node) not in written_edges:
                    f.write(f"{node_1} {neighbor_1}\n")
                    written_edges.add((node, neighbor))

        f.write("-1\n")
        f.write("EOF\n")

    print("Saved")


# Save a solution in TSBLIB format.
def save_tour_file(tour, filename="undefined", name="Undefined"):
    output_dir = "HCP_results"

    if not filename.endswith(".tour"):
        filename += ".tour"

    filepath = os.path.join(output_dir, filename)

    num_nodes = len(tour)

    with open(filepath, "w") as f:
        f.write(f"NAME : {name}\n")
        f.write("TYPE : HCP TOUR\n")
        f.write(f"COMMENT : {num_nodes}-node graph\n")
        f.write(f"DIMENSION : {num_nodes}\n")
        f.write("EDGE_DATA_FORMAT : EDGE_LIST\n")
        f.write("EDGE_DATA_SECTION\n")

        for node in tour:
            f.write(f"{node}\n")

        f.write("-1\n")
        f.write("EOF\n")
