addrs= [0x000055c5593bf000,
0x000055c5596c6000,
0x000055c5596ca000,
0x000055c5596d3000,
0x000055c55977f000,
0x00007f4e907ca000,
0x00007f4e907d5000,
0x00007f4e909d5000,
0x00007f4e909d6000,
0x00007f4e909d7000,
0x00007f4e909dd000,
0x00007f4e9107e000,
0x00007f4e9e034000,
0x00007f4e9e201000,
0x00007f4e9e400000,
0x00007f4e9e404000,
0x00007f4e9e406000,
0x00007f4e9e40a000,
0x00007f4e9e40d000,
0x00007f4e9e60c000,
0x00007f4e9e60d000,
0x00007f4e9e60e000,
0x00007f4e9e637000,
0x00007f4e9e836000,
0x00007f4e9e83a000,
0x00007f4e9e83b000,
0x00007f4e9ea01000,
0x00007f4e9ea54000,
0x00007f4e9ea55000,
0x00007f4e9ea56000,
0x00007f4e9ea57000,
0x00007f4e9ea58000,
0x00007f4e9ea59000,
0x00007f4e9ea5e000,
0x00007f4e9ea5f000,
0x00007f4e9ea60000,
0x00007f4e9ea61000,
0x00007f4e9ea62000,
0x00007f4e9ea69000,
0x00007f4e9ea6a000,
0x00007f4e9ea6b000,
0x00007ffd68556000,
0x00007ffd68591000,
0x00007ffd68595000]

d = {}
mask_9bit = (1 << 9) - 1  # 0x1FF (511)

for i in addrs:
    # 1. Isolate the lower 48 bits of the address
    addr = i & ((1 << 48) - 1)

    # 2. Shift and mask to extract each 9-bit index
    # Level 4: bits 39-47 (shift right by 39)
    l4_idx = (addr >> 39) & mask_9bit
    
    # Level 3: bits 30-38 (shift right by 30)
    l3_idx = (addr >> 30) & mask_9bit
    
    # Level 2: bits 21-29 (shift right by 21)
    l2_idx = (addr >> 21) & mask_9bit
    
    # Level 1: bits 12-20 (shift right by 12)
    l1_idx = (addr >> 12) & mask_9bit

    # 3. Build the hierarchical page table safely
    # setdefault checks if the key exists; if not, it creates it with {}
    l4_table = d.setdefault(l4_idx, {})
    l3_table = l4_table.setdefault(l3_idx, {})
    l2_table = l3_table.setdefault(l2_idx, {})
    
    # At Level 1, we map to a physical page frame (represented here as a string or 1)
    l2_table[l1_idx] = ""

for l4 in d:
    print(f"[CR3 Root] -> Level 4 Table [Index {l4}]")
    for l3 in d[l4]:
        print(f" ├── Level 3 Table [Index {l3}]")
        for l2 in d[l4][l3]:
            print(f" │    ├── Level 2 Table [Index {l2}]")
            for l1 in d[l4][l3][l2]:
                status = d[l4][l3][l2][l1]
                print(f" │    │    └── Level 1 Table [Index {l1}] ──> [ {status} ]")
    print() # Add a blank line between root branches
