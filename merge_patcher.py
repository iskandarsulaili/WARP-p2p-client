
with open("launcher_b64.txt", "r") as f:
    b64_data = f.read()

with open("patcher.py", "r") as f:
    script_content = f.read()

# Replace the dummy B64 block with the real one
# I'll look for the start and end markers in the script I just wrote
start_marker = 'LAUNCHER_B64 = """'
end_marker = '"""'

# Construct new content
# Note: My previous write_to_file put a partial B64 string.
# I will replace the whole variable definition.

new_content = script_content.split(start_marker)[0] + start_marker + "\n" + b64_data + "\n" + end_marker + script_content.split(end_marker, 1)[1]

with open("patcher.py", "w") as f:
    f.write(new_content)
