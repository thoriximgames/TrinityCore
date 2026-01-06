import struct

def create_map_dbc(output_path, maps):
    """
    Generates a valid Map.dbc for TrinityCore 3.3.5a
    maps: List of dictionaries containing map data
    """
    # WDBC Header: Magic, RecordCount, FieldCount, RecordSize, StringSize
    MAGIC = b'WDBC'
    FIELD_COUNT = 17
    RECORD_SIZE = FIELD_COUNT * 4
    
    records = []
    string_block = b'\x00' # First byte is always null
    string_map = {}

    def get_string_offset(s):
        nonlocal string_block
        if not s: return 0
        if s in string_map: return string_map[s]
        
        offset = len(string_block)
        string_map[s] = offset
        string_block += s.encode('utf-8') + b'\x00'
        return offset

    for m in maps:
        # Field 1: ID
        # Field 2: Internal Name (String)
        # Field 3: Instance Type (0: World, 1: Inst, 2: Raid, 3: BG, 4: Arena)
        # Field 4: IsPVP (0/1)
        # Field 5: Public Name (String)
        # Field 15: Expansion (0: Vanilla, 1: BC, 2: WotLK)
        
        record = struct.pack('<i i i i i i i i i f i f f i i i i',
            m.get('id', 0),
            get_string_offset(m.get('internal_name', 'CustomMap')),
            m.get('type', 0),
            m.get('is_pvp', 0),
            get_string_offset(m.get('name', 'Custom Map')),
            m.get('area_id', 0), # Field 6: AreaTable
            0, 0, # Field 7, 8: Descriptions
            m.get('loading_screen', 0), # Field 9
            1.0, # Field 10: Minimap scale
            m.get('corpse_map', -1), # Field 11: Corpse Map
            0.0, 0.0, # Field 12, 13: Corpse X/Y
            -1, # Field 14: Time of day
            m.get('expansion', 2), # Field 15
            0, # Field 16: Raid service
            m.get('max_players', 0) # Field 17
        )
        records.append(record)

    with open(output_path, 'wb') as f:
        f.write(MAGIC)
        f.write(struct.pack('<I', len(records)))
        f.write(struct.pack('<I', FIELD_COUNT))
        f.write(struct.pack('<I', RECORD_SIZE))
        f.write(struct.pack('<I', len(string_block)))
        for r in records:
            f.write(r)
        f.write(string_block)

# --- CONFIGURATION ---
custom_maps = [
    {'id': 0, 'internal_name': 'Azeroth', 'name': 'Eastern Kingdoms'},
    {'id': 1, 'internal_name': 'Kalimdor', 'name': 'Kalimdor'},
    {'id': 530, 'internal_name': 'Expansion01', 'name': 'Outland'},
    {'id': 571, 'internal_name': 'Northrend', 'name': 'Northrend'},
    # YOUR NEW MAP
    {'id': 1000, 'internal_name': 'MyNewMap', 'name': 'The Shattered Isles', 'type': 0}
]

output = "MMO-Server/MyMMOServer/bin/data/dbc/Map.dbc"
create_map_dbc(output, custom_maps)
print(f"Successfully generated {output} with {len(custom_maps)} maps.")
