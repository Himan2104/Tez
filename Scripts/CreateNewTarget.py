import os
import sys
from pathlib import Path

def create_project_structure(target_name):
    # Root folder for the new target
    base_path = Path(target_name)
    
    # Define the sub-modules
    modules = ["Runtime", "Editor"]
    
    # Define the internal directory structure
    # Structure: Module -> Include/Source -> Public/Private -> Tez/TargetName
    for module in modules:
        for folder_type in ["Source", "Include"]:
            if folder_type == "Include":
                for visibility in ["Public", "Private"]:
                    # Create the namespaced path: Target/Module/Include/Visibility/Tez/TargetName
                    path = base_path / module / folder_type / visibility / "Tez" / target_name
                    path.mkdir(parents=True, exist_ok=True)
            else:
                # Create the source path: Target/Module/Source
                path = base_path / module / "Source"
                path.mkdir(parents=True, exist_ok=True)

    # Create the root CMakeLists.txt
    cmake_content = f"""# CMakeLists.txt for {target_name}
project({target_name} LANGUAGES CXX)

# Add your subdirectories or target logic here
# add_subdirectory(Runtime)
# add_subdirectory(Editor)
"""
    
    with open(base_path / "CMakeLists.txt", "w") as f:
        f.write(cmake_content)

    print(f"Successfully created target structure for: {target_name}")
    print(f"Location: {os.path.abspath(target_name)}")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        name = input("Enter the name of the new target: ")
    else:
        name = sys.argv[1]
    
    if name:
        create_project_structure(name)
    else:
        print("Error: Target name cannot be empty.")
