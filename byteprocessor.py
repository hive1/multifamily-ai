import csv
import io
import struct

import pandas as pd

# Layout of mapOutput.txt (little-endian)
#   header : int32 x squares | int32 y squares | float64 square size (m)
#   record : int32 zone type, then float64 each for
#            utility access, pop factor, employment factor,
#            job quantity, rent factor, quality
HEADER = struct.Struct("<iid")
RECORD = struct.Struct("<i6d")

COLUMNS = ["zone", "utility", "pop_factor", "employment", "jobs", "rent", "quality"]


def load_grid(source="mapOutput.txt"):
    """Read the grid file. `source` is a file path or the raw bytes of the file.

    Returns (x_squares, y_squares, square_size_m, DataFrame).
    The DataFrame has one row per square:
        col, row      grid indices (0,0 = first square)
        x_m, y_m      offset of the square's corner from the grid origin, in metres
        zone, utility, pop_factor, employment, jobs, rent, quality
    """
    if isinstance(source, (bytes, bytearray)):
        f = io.BytesIO(source)
    else:
        f = open(source, "rb")

    with f:
        int1, int2, squareSize = HEADER.unpack(f.read(HEADER.size))  # x squares, y squares, size
        totalGridSquares = int1 * int2

        rows = []
        for i in range(totalGridSquares):
            chunk = f.read(RECORD.size)
            if len(chunk) != RECORD.size:
                raise ValueError(
                    f"File ended at square {i} of {totalGridSquares} (a {int1}x{int2} grid)."
                )
            zoneType, utility, popFactor, employment, job, rent, quality = RECORD.unpack(chunk)

            col = i % int1
            row = i // int1
            squareX = squareSize * col
            squareY = squareSize * row

            rows.append((col, row, squareX, squareY, zoneType, utility, popFactor, employment, job, rent, quality))

    df = pd.DataFrame(rows, columns=["col", "row", "x_m", "y_m"] + COLUMNS)
    return int1, int2, squareSize, df


if __name__ == "__main__":
    nx, ny, size, df = load_grid("mapOutput.txt")
    print(f"{nx} x {ny} squares, {size} m each ({len(df)} total)")
    print(df.head())

    with open("mapOutput.csv", "w", newline="") as out:
        writer = csv.writer(out)
        writer.writerow(df.columns)
        writer.writerows(df.itertuples(index=False))
    print("Wrote mapOutput.csv")
