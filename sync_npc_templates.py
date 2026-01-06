import json
import sys
import os
import subprocess

def get_npc_templates(db_bin_dir, db_user, db_pass, db_name, output_json):
    # Query: Select Entry, Name, SubName, ModelId1 from creature_template
    query = "SELECT entry, name, subname, modelid1, minlevel, maxlevel, faction FROM creature_template;"
    
    cmd = [
        os.path.join(db_bin_dir, "mariadb.exe"),
        f"-u{db_user}",
        f"-p{db_pass}",
        "-e", query,
        "--batch", # Tab separated
        db_name
    ]
    
    try:
        result = subprocess.run(cmd, capture_output=True, text=True)
        if result.returncode != 0:
            print(f"DB Error: {result.stderr}")
            return

        lines = result.stdout.strip().split('\n')
        if len(lines) < 2: 
            print("No NPCs found.")
            return

        headers = lines[0].split('\t')
        npc_list = []

        for line in lines[1:]:
            values = line.split('\t')
            npc = dict(zip(headers, values))
            npc_list.append(npc)

        with open(output_json, 'w') as f:
            json.dump({"templates": npc_list}, f, indent=4)
        
        print(f"Exported {len(npc_list)} NPC templates to {output_json}")

    except Exception as e:
        print(f"Sync failed: {str(e)}")

if __name__ == "__main__":
    if len(sys.argv) < 6:
        print("Usage: python sync_npc_templates.py <db_bin> <user> <pass> <db> <output_json>")
        sys.exit(1)

    db_bin, user, password, db, output = sys.argv[1:6]
    get_npc_templates(db_bin, user, password, db, output)
