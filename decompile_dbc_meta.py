import struct
import os

def analyze_dbc(file_path):
    if not os.path.exists(file_path):
        print(f"Error: {file_path} not found.")
        return

    with open(file_path, 'rb') as f:
        header = f.read(20)
        if len(header) < 20: return
        
        magic, record_count, field_count, record_size, string_size = struct.unpack('<4sIIII', header)
        print(f"--- Analysis of {os.path.basename(file_path)} ---")
        print(f"Magic: {magic}")
        print(f"Records: {record_count}")
        print(f"Fields: {field_count}")
        print(f"Record Size: {record_size} bytes")
        print(f"String Block Size: {string_size} bytes")
        
        # Check if field_count * 4 matches record_size
        if field_count * 4 != record_size:
            print(f"Note: This file uses non-standard field sizes (likely 8-byte types).")

# Analyze Map.dbc (the one I made) and another original one to compare
analyze_dbc("MMO-Server/MyMMOServer/bin/data/dbc/Map.dbc")
analyze_dbc("MMO-Server/MyMMOServer/bin/data/dbc/AreaTable.dbc") # An original Blizzard one
