import tkinter as tk
import time

from solver import (
    parse_tsp_file,
    parse_tour_file
)


FILEPATH = "TSP_instances/xqf131.tsp"
TOUR_FILEPATH = "TSP_results/xqf131_1.tour"


def generate_canvas(points, tour):
    root = tk.Tk()
    root.title("Visualize Tour")

    canvas_width = 800
    canvas_height = 500

    canvas = tk.Canvas(root, width=canvas_width, height=canvas_height, bg="white")
    canvas.pack(fill=tk.BOTH, expand=True)

    button = tk.Button(root, text="Generate TSP instance",
                       command=lambda: generate_tsp_points(canvas, points))
    button.pack(side="left")

    button2 = tk.Button(root, text="Make the tour",
                        command=lambda: make_tour(canvas, points, tour))
    button2.pack(side="right")

    root.mainloop()


def update_tkinter(canvas, time_interval=0.01):
    canvas.update_idletasks()
    canvas.update()
    time.sleep(time_interval)


def connect_points(canvas, point_1, point_2, color=None, size=None):
    canvas.create_line(point_1[0], point_1[1],
                       point_2[0], point_2[1],
                       fill=color, width=size)


def make_tour(canvas, points, tour):
    for idx in range(len(tour) - 1):
        point1 = tour[idx]
        point2 = tour[idx + 1]

        initial_point = points[point1]
        target_point = points[point2]

        connect_points(canvas, initial_point, target_point, color="red", size=3)
        update_tkinter(canvas)

    connect_points(canvas, points[tour[-1]], points[tour[0]], color="red", size=3)


def generate_tsp_points(canvas, points):
    canvas.delete("all")
    canvas.update()

    canvas_width = 800
    canvas_height = 500
    padding = 50

    x_coords = [p[0] for p in points]
    y_coords = [p[1] for p in points]
    min_x, max_x = min(x_coords), max(x_coords)
    min_y, max_y = min(y_coords), max(y_coords)

    width = max_x - min_x
    height = max_y - min_y

    width = max(width, 1e-6)
    height = max(height, 1e-6)

    scale_x = (canvas_width - 2 * padding) / width
    scale_y = (canvas_height - 2 * padding) / height
    scale = min(scale_x, scale_y)

    offset_x = (canvas_width - width * scale) / 2 - min_x * scale
    offset_y = (canvas_height - height * scale) / 2 - min_y * scale

    for idx in range(len(points)):
        x = points[idx][0] * scale + offset_x
        y = points[idx][1] * scale + offset_y
        points[idx] = (x, y)
        canvas.create_oval(x - 5, y - 5, x + 5, y + 5, fill="blue")


def main():
    points = parse_tsp_file(FILEPATH)
    tour = parse_tour_file(TOUR_FILEPATH)
    generate_canvas(points, tour)


if __name__ == "__main__":
    main()
