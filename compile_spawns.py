import json
import sys
import os
import subprocess

def generate_spawn_sql(manifest_path, output_sql):
    with open(manifest_path, 'r') as f:
        data = json.load(f)
        spawns = data.get('spawns', [])

    sql_lines = [
        "USE world;",
        "-- Unity Exported Spawns",
    ]

    for s in spawns:
        # WoW X = Unity Z, WoW Y = -Unity X, WoW Z = Unity Y
        x = s['pos_z']
        y = -s['pos_x']
        z = s['pos_y']
        o = s['rot_y']
        
        # Updated columns based on DESCRIBE creature:
        # guid, id, map, zoneId, areaId, spawnMask, phaseMask, position_x, position_y, position_z, orientation, spawntimesecs, wander_distance, MovementType
        sql = f"REPLACE INTO creature (guid, id, map, zoneId, areaId, spawnMask, phaseMask, position_x, position_y, position_z, orientation, spawntimesecs, wander_distance, MovementType) " \
              f"VALUES ({s['guid']}, {s['entry']}, {s['map']}, 0, 0, 1, 1, {x:.4f}, {y:.4f}, {z:.4f}, {o:.4f}, 300, 0, 0);"
        sql_lines.append(sql)

    with open(output_sql, 'w') as f:
        f.write("\n".join(sql_lines))

def execute_sql(sql_file, db_bin_dir, db_user, db_pass, db_name):
    abs_bin_dir = os.path.abspath(db_bin_dir)
    exe_path = os.path.join(abs_bin_dir, "mariadb.exe")
    
    cmd = [
        exe_path,
        f"-u{db_user}",
        f"-p{db_pass}",
        db_name
    ]
    
    try:
        with open(sql_file, 'r') as f:
            result = subprocess.run(cmd, stdin=f, capture_output=True, text=True)
            if result.returncode == 0:
                print("SQL Execution Successful.")
            else:
                print(f"SQL Error: {result.stderr}")
    except Exception as e:
        print(f"Execution failed: {str(e)}")

if __name__ == "__main__":
    if len(sys.argv) < 7:
        sys.exit(1)

    manifest, output, db_bin, user, password, db = sys.argv[1:7]
    generate_spawn_sql(manifest, output)
    execute_sql(output, db_bin, user, password, db)
