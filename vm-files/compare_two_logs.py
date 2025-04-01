import argparse
import re
import collections
from pathlib import Path

class Log_achyb_report():
    def __init__(self, log_line: str):
        '''pre: log_line is of the form "a:b:c\n"'''
        self.reported = log_line.split(':')[0]
        self.protected = log_line.split(':')[1]
        self.evidence = log_line.split(':')[2][:-1]
    
    def __repr__(self):
        return f"{self.reported}:{self.protected}:{self.evidence}"

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

    file1_reports = get_reports(file1_content)
    file2_reports = get_reports(file2_content)
    
    file1_reported_funcs = set(report.reported for report in file1_reports)
    file2_reported_funcs = set(report.reported for report in file2_reports)

    size_diff = len(file1_reported_funcs) - len(file2_reported_funcs) 
    if(size_diff > 0):
        print(f"{file1} has {size_diff} more reported functions than {file2}")
    elif(size_diff < 0):
        print(f"{file1} has {-size_diff} less reported functions than {file2}")
    else:
        print(f"Both files have {len(file1_reported_funcs)} reports")

    print_difference_between(file1, file2, file1_reports, file2_reports)
    print_difference_between(file2, file1, file2_reports, file1_reports)

# FIXME unused and untested function
def print_details_of(file: Path, reports: list[Log_achyb_report]) -> None:
    reported_name_info = collections.Counter(report.reported for report in reports)
    unique_name_repeated = dict(pair for pair in reported_name_info.items() if pair[1] > 1)
    unique_name_only_once = [pair[0] for pair in reported_name_info.items() if pair[1] == 1]
    print(f"###\n Number of reports in {file}:\n\t{len(reports)}")
    print(f"Number of unique function names in reported functions:\n\t{len(unique_name_only_once)}")
    print(f"Number of repeated function names in reported functions:\n\t{len(unique_name_repeated)}")
    print(f"The functions reported more than once are:")
    for name, count in unique_name_repeated.items():
        print(f"\t{name} ---- {count}")

def print_difference_between(f1: Path,f2: Path, f1_reports: list[Log_achyb_report], f2_reports: list[Log_achyb_report]) -> None: 
    f1_reported_funcs = set(report.reported for report in f1_reports)
    f2_reported_funcs = set(report.reported for report in f2_reports)
    print(f"###\n Functions reported in {f1} but not in {f2} are:\n")
    if (f1_reported_funcs - f2_reported_funcs):
        for fun in (f1_reported_funcs - f2_reported_funcs):
            print('\t'+fun)
    else:
        print("None")
    print("\n###")

def get_reports(file_contents: list[str]) -> list[Log_achyb_report]:
    idx = file_contents.index("Results:\n")
    interesting_lines = file_contents[idx+1:]
    assert all([not(re.match(r".+:.+:.+\n", line)) for line in interesting_lines[-4:]]) , "We have too few extra lines at the end, something went wrong"
    assert re.match(r".+:.+:.+\n", interesting_lines[-5]), "Fourth to last line does not match r'.+:.+:.+\n', something went wrong "
 
    return [Log_achyb_report(line) for line in interesting_lines[:-4]]

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
