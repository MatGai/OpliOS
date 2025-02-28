import os

def find_include_dirs(root):
    include_dirs = set()
    for dirpath, _, _ in os.walk(root):
        # Check if the directory name contains "include" (case-insensitive)
        if "include" in os.path.basename(dirpath).lower():
            include_dirs.add(dirpath)
    return include_dirs

# Assume the script is run from the project (bootloader) directory.
project_dir = os.getcwd()

# Set the base directory to edk2 (adjust if needed)
edk2_dir = os.path.join(project_dir, "edk2")

include_dirs = find_include_dirs(edk2_dir)

relative_paths = []
for full_path in sorted(include_dirs):
    # Compute the relative path with respect to the project directory.
    rel = os.path.relpath(full_path, project_dir)
    # Replace backslashes with forward slashes.
    rel = rel.replace("\\", "/")
    # Prepend with $(ProjectDir)
    relative_paths.append("$(ProjectDir)" + rel)

# Join the paths using semicolons.
include_dirs_string = ";".join(relative_paths)

# Write the output to a file in the project directory.
output_file = os.path.join(project_dir, "include_dirs.txt")
with open(output_file, "w") as f:
    f.write(include_dirs_string)

print(f"Include directories logged to {output_file}")
