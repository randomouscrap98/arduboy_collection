# Function to generate the sequence using the LCG formula
def lcg(x):
    return (0x41 * x + 0x1F) & 0xFF


# Generate and print all 256 values of the sequence
for x in range(256):
    print(format(lcg(x), '08b'))
