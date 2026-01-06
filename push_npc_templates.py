import json
import sys
import os
import subprocess

def generate_template_sql(manifest_path, output_sql):
    with open(manifest_path, 'r') as f:
        data = json.load(f)
        templates = data.get('templates', [])

    sql_lines = [
        "USE world;",
        "-- Unity Exported NPC Templates",
    ]

    for t in templates:
        # We use REPLACE INTO to overwrite or create
        sql = (f"REPLACE INTO creature_template (entry, name, subname, modelid1, scale, minlevel, maxlevel, "
               f"faction, minhealth, maxhealth, minmana, maxmana, speed_walk, speed_run, AIName, ScriptName, npcflag, unit_flags) "
               f"VALUES ({t['entry']}, '{t['name']}', '{t['subname']}', {t['modelid1']}, {t['scale']}, {t['minLevel']}, {t['maxLevel']}, "
               f"{t['faction']}, {t['minHealth']}, {t['maxHealth']}, {t['minMana']}, {t['maxMana']}, {t['speedWalk']}, {t['speedRun']}, "
               f"'{t['aiName']}', '{t['scriptName']}', {t['npcFlag']}, {t['unitFlags']}");")
        sql_lines.append(sql)

    with open(output_sql, 'w') as f:
        f.write("\n".join(sql_lines))

def execute_sql(sql_file, db_bin_dir, db_user, db_pass, db_name):
    abs_bin_dir = os.path.abspath(db_bin_dir)
    exe_path = os.path.join(abs_bin_dir, "mariadb.exe")
    cmd = [exe_path, f"-u{db_user}", f"-p{db_pass}", db_name]
    try:
        with open(sql_file, 'r') as f:
            result = subprocess.run(cmd, stdin=f, capture_output=True, text=True)
            if result.returncode == 0:
                print("Template SQL Execution Successful.")
            else:
                print(f"SQL Error: {result.stderr}")
    except Exception as e:
        print(f"Execution failed: {str(e)}")

if __name__ == "__main__":
    if len(sys.argv) < 7:
        sys.exit(1)
    manifest, output, db_bin, user, password, db = sys.argv[1:7]
    generate_template_sql(manifest, output)
    execute_sql(output, db_bin, user, password, db)
