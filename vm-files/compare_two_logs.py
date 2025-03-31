import argparse
import sys
from pathlib import Path
import re


def try_open_file(filepath: Path) -> list[str]:
    try:
        with open(filepath, 'r') as f:
            lines = f.readlines()
    except OSError as e:
        print(f"Cannot open {filepath}. Check your paths and permissions.")
        raise(e)
    return lines


def compare_files(file1: Path, file2: Path) -> None:
    file1_content = try_open_file(file1)
    file2_content = try_open_file(file2)

    file1_reported_funcs = set(get_reported_funcs(file1_content))
    file2_reported_funcs = set(get_reported_funcs(file2_content))
    
    size_diff = len(file1_reported_funcs) - len(file2_reported_funcs) 
    if(size_diff > 0):
        print(f"{file1} has {size_diff} more reports than {file2}")
    elif(size_diff < 0):
        print(f"{file1} has {-size_diff} less reports than {file2}")
    else:
        print(f"Both files have {len(file1_reported_funcs)} reports")

    print_difference_between(file1, file2, file1_reported_funcs, file2_reported_funcs)
    print_difference_between(file2, file1, file2_reported_funcs, file1_reported_funcs)

def print_difference_between(f1: Path,f2: Path,f1_reports: set[str], f2_reports: set[str]) -> None: 
    print(f"###\n Reports present in {f1} but not in {f2} are:")
    if (f1_reports - f2_reports):
        print(*(f1_reports - f2_reports))
    else:
        print("None")
    print("###")

def get_reported_funcs(file_contents: list[str]) -> list[str]:
    idx = file_contents.index("Results:\n")
    interesting_lines = file_contents[idx+1:]
    assert all([not(re.match(r".+:.+:.+\n", line)) for line in interesting_lines[-4:]]) , "We have too few extra lines at the end, something went wrong"
    assert re.match(r".+:.+:.+\n", interesting_lines[-5]), "Fourth to last line does not match r'.+:.+:.+\n', something went wrong "
 
    return interesting_lines[:-4]

def main():
    parser = argparse.ArgumentParser(description="Compare two log files")
    parser.add_argument("file1", nargs="?", help="Path to the first log file")
    parser.add_argument("file2", nargs="?", help="Path to the second log file")
    args = parser.parse_args()

    if (args.file1 is None) or (args.file2 is None):
        parser.print_help()
        parser.exit()

    else:
        args.file1 = Path(args.file1).resolve()
        args.file2 = Path(args.file2).resolve()

    compare_files(args.file1, args.file2)

if __name__ == "__main__":
    main()
