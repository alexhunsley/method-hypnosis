# gen_points_for_place_notation.py

def apply_place_notation(row, notation):
    """Apply a single place notation step to a row."""
    if notation == 'x':
        # Cross: swap all adjacent pairs
        return ''.join([row[i + 1] + row[i] for i in range(0, len(row), 2)])
    else:
        # Make places, swap all other pairs
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


def parse_place_notation_sequence(seq):
    result = []
    current = ""
    final_result = []
    
    for char in seq:
        if char == ',':
            if current:
                result.append(current)
                current = ""
            final_result.extend(result)
            final_result.extend(list(reversed(result))[1:])
            result = []
        elif char in ".x":
            if current:
                result.append(current)
                current = ""
            if char == 'x':
                result.append('x')
        else:
            current += char

    if current:
        result.append(current)

    final_result.extend(result)
    return final_result
    

def generate_rows(start_row, place_notation_string):
    """Generate the sequence of rows based on parsed place notation."""
    sequence = parse_place_notation_sequence(place_notation_string)

    print(f"processed PN: {sequence}")

    row = start_row
    rows = [row]
    for pn in sequence:
        row = apply_place_notation(row, pn)
        rows.append(row)
    return rows


# Full place notation for one lead of Bristol Surprise Major
place_notation_str = "x58x14.58x58.36.14x14.58x14x18,18"
# place_notation_str = "x12,14"
sequence = parse_place_notation_sequence(place_notation_str)

print(sequence)
print(len(sequence))

# Generate rows
rows = generate_rows("12345678", place_notation_str)
print("\n".join(rows))


# # Extract positions of bell 8 (zero-indexed)
# positions = bell_8_positions(rows)

# # Output as C array
# print("const uint8_t y_points[] = { " + ', '.join(map(str, positions)) + " };")