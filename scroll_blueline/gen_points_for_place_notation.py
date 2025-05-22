# gen_points_for_place_notation.py

def apply_place_notation(row, notation):
    """
    Applies a single place notation to a row.
    """
    if notation == "-":
        # Swap adjacent pairs (cross)
        return ''.join([row[i + 1] + row[i] for i in range(0, len(row), 2)])
    else:
        # Place notation: make places at the specified positions, swap others
        places = [int(ch) - 1 for ch in notation if ch.isdigit()]
        new_row = list(row)
        i = 0
        while i < len(row) - 1:
            if i in places:
                i += 1
                continue
            new_row[i], new_row[i + 1] = new_row[i + 1], new_row[i]
            i += 2
        return ''.join(new_row)

def generate_rows(start_row, place_notations):
    """
    Generate rows by applying place notations in sequence.
    """
    row = start_row
    rows = [row]
    for pn in place_notations:
        row = apply_place_notation(row, pn)
        rows.append(row)
    return rows

def bell_8_positions(rows):
    """
    Extract zero-based positions of bell '8' from each row.
    """
    return [row.index('8') for row in rows]

# Example Place Notation for Bristol Surprise Major (partial, for demo purposes)
place_notation_str = "-58-14-58-36-14-58-36-14-58-14-58-36-14-58-36-18"
place_notations = place_notation_str.split('-')

# Generate rows
rows = generate_rows("12345678", place_notations)
print('\n'.join(rows))

# # Extract 8's positions (zero-indexed)
# positions = bell_8_positions(rows)

# # Output as C array
# print("const uint8_t y_points[] = { " + ', '.join(map(str, positions)) + " };")
