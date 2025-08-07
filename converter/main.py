import argparse
import os
from parser import CParser
from generator import LuaGenerator
from merger import merge_lua_files

def process_single_file(input_path, output_dir):
    """Processes a single C file and generates a corresponding Lua file."""
    base_name = os.path.basename(input_path)
    file_name, _ = os.path.splitext(base_name)
    output_path = os.path.join(output_dir, f"{file_name}.lua")

    print(f"Processing {input_path} -> {output_path}")

    try:
        with open(input_path, 'r', encoding='utf-8') as f:
            c_code = f.read()
    except FileNotFoundError:
        print(f"Error: Input file not found at {input_path}")
        return False
    except Exception as e:
        print(f"Error reading file {input_path}: {e}")
        return False

    c_parser = CParser(c_code)
    ui_elements = c_parser.parse()

    lua_generator = LuaGenerator(ui_elements)
    lua_code = lua_generator.generate()

    try:
        with open(output_path, 'w', encoding='utf-8') as f:
            f.write(lua_code)
        print(f"Successfully converted {base_name}")
        return True
    except Exception as e:
        print(f"Error writing to output file {output_path}: {e}")
        return False

def main():
    """
    Main function to parse command line arguments and run the converter.
    """
    parser = argparse.ArgumentParser(description="Convert GUI-Guider C code to a single merged Lua file.")
    parser.add_argument("input_dir", help="Directory containing the input C files.")
    parser.add_argument("-o", "--output", default="converter/generated/generated_ui.lua", help="Path to the final merged output Lua file.")
    parser.add_argument("--temp-dir", default="converter/generated/temp", help="Directory to store intermediate Lua files.")
    args = parser.parse_args()

    input_dir = args.input_dir
    output_file = args.output
    temp_dir = args.temp_dir

    # Ensure the temp and final output directories exist
    os.makedirs(temp_dir, exist_ok=True)
    os.makedirs(os.path.dirname(output_file), exist_ok=True)

    c_files = [f for f in os.listdir(input_dir) if f.endswith('.c')]
    if not c_files:
        print(f"No .c files found in {input_dir}")
        return

    success_count = 0
    for c_file in c_files:
        input_path = os.path.join(input_dir, c_file)
        if process_single_file(input_path, temp_dir):
            success_count += 1

    if success_count > 0:
        print(f"\nStarting merge process for {success_count} generated files...")
        merge_lua_files(temp_dir, output_file)
    else:
        print("No files were successfully converted, skipping merge step.")


if __name__ == "__main__":
    main()