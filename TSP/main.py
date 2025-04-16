from parse_tsp import (
    parse_tsp_file,
    parse_tour_file
)

from solver import (
    solve_tsp,
    calculate_tour_distance
)


from graphics import (
    generate_canvas
)


def main():
    tsp_filepath = "TSP_instances/ch130.tsp"
    tsp_points = parse_tsp_file(tsp_filepath)

    tsp_tour_filepath = "TSP_instances/ch130.opt.tour"
    opt_tour = parse_tour_file(tsp_tour_filepath)

    instance_name = tsp_filepath.split("/")[-1].split(".")[0]
    tour, dist = solve_tsp(tsp_points, instance_name)

    opt_dist = calculate_tour_distance(tsp_points, opt_tour)

    print(f"Optimal distance: {opt_dist}")
    print(f"Best found distance: {dist}")


def main_graphics():
    tsp_filepath = "TSP_instances/xit1083.tsp"
    tsp_points = parse_tsp_file(tsp_filepath)

    tsp_tour_filepath = "TSP_results/xit1083_2.tour"
    tour = parse_tour_file(tsp_tour_filepath)

    generate_canvas(tsp_points, tour)


if __name__ == "__main__":
    main()
