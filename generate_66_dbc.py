import struct
import json
import sys
import os

def create_66_field_map_dbc(output_path, custom_maps_data):
    MAGIC = b'WDBC'
    FIELD_COUNT = 66
    RECORD_SIZE = FIELD_COUNT * 4
    
    records_dict = {} # Use dict to manage overrides
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

    # 1. FILL MANDATORY BLIZZARD SLOTS (0-730)
    # The server crashes if these are missing because other DBCs reference them.
    for i in range(731):
        name = "LegacyMap"
        if i == 0: name = "Azeroth"
        if i == 1: name = "Kalimdor"
        if i == 530: name = "Outland"
        if i == 571: name = "Northrend"
        
        fields = [0] * FIELD_COUNT
        fields[0] = i
        fields[1] = get_string_offset(name)
        fields[2] = 0
        fields[4] = get_string_offset(name)
        fields[15] = 2
        records_dict[i] = fields

    # 2. OVERWRITE WITH CUSTOM UNITY DATA
    for m in custom_maps_data:
        m_id = m.get('id', 0)
        fields = [0] * FIELD_COUNT
        fields[0] = m_id
        fields[1] = get_string_offset(m.get('internal_name', 'CustomMap'))
        fields[2] = m.get('type', 0)
        fields[4] = get_string_offset(m.get('name', 'Custom Map'))
        fields[15] = m.get('expansion', 2)
        records_dict[m_id] = fields

    # 3. SORT BY ID (Crucial for TrinityCore)
    sorted_ids = sorted(records_dict.keys())
    final_records = []
    for m_id in sorted_ids:
        record = struct.pack('<' + 'i' * FIELD_COUNT, *records_dict[m_id])
        final_records.append(record)

    with open(output_path, 'wb') as f:
        f.write(MAGIC)
        f.write(struct.pack('<I', len(final_records)))
        f.write(struct.pack('<I', FIELD_COUNT))
        f.write(struct.pack('<I', RECORD_SIZE))
        f.write(struct.pack('<I', len(string_block)))
        for r in final_records: f.write(r)
        f.write(string_block)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        sys.exit(1)

    manifest_path = sys.argv[1]
    output_file = sys.argv[2]

    try:
        with open(manifest_path, 'r') as f:
            data = json.load(f)
            custom_maps = data.get('maps', [])
        
        create_66_field_map_dbc(output_file, custom_maps)
        print(f"Successfully compiled HYBRID Map.dbc with {len(custom_maps)} custom maps.")
    except Exception as e:
        print(f"Error: {str(e)}")
        sys.exit(1)