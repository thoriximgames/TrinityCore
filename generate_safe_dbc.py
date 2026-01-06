import struct

def create_safe_map_dbc(output_path, custom_map_id, custom_map_name):
    MAGIC = b'WDBC'
    FIELD_COUNT = 17
    RECORD_SIZE = FIELD_COUNT * 4
    
    records = []
    string_block = b'\x00'
    string_map = {}

    def get_string_offset(s):
        nonlocal string_block
        if not s: return 0
        if s in string_map: return string_map[s]
        offset = len(string_block)
        string_map[s] = offset
        string_block += s.encode('utf-8') + b'\x00'
        return offset

    # We need to fill IDs from 0 to 720 to satisfy most WotLK foreign keys
    # We'll mark them as 'Reserved' so the server doesn't crash on Achievement/Spell lookups
    for i in range(730):
        name = "Reserved"
        if i == 0: name = "Azeroth"
        if i == 1: name = "Kalimdor"
        if i == 530: name = "Outland"
        if i == 571: name = "Northrend"
        
        record = struct.pack('<i i i i i i i i i f i f f i i i i',
            i, get_string_offset(name), 0, 0, get_string_offset(name), 
            0, 0, 0, 0, 1.0, -1, 0.0, 0.0, -1, 2, 0, 0
        )
        records.append(record)

    # ADD YOUR CUSTOM MAP
    record = struct.pack('<i i i i i i i i i f i f f i i i i',
        custom_map_id, get_string_offset("CustomMap"), 0, 0, get_string_offset(custom_map_name), 
        0, 0, 0, 0, 1.0, -1, 0.0, 0.0, -1, 2, 0, 0
    )
    records.append(record)

    with open(output_path, 'wb') as f:
        f.write(MAGIC)
        f.write(struct.pack('<I', len(records)))
        f.write(struct.pack('<I', FIELD_COUNT))
        f.write(struct.pack('<I', RECORD_SIZE))
        f.write(struct.pack('<I', len(string_block)))
        for r in records: f.write(r)
        f.write(string_block)

output = "MMO-Server/MyMMOServer/bin/data/dbc/Map.dbc"
create_safe_map_dbc(output, 1000, "The Shattered Isles")
print(f"Generated SAFE Map.dbc with {730} reserved slots and Map 1000.")
