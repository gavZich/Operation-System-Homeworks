import sys
import os
import shutil
import subprocess
import time
import filecmp

EXECUTABLE = './file_sync'


def run(cmd):
    """Run a command and return (stdout + stderr, exit_code)."""
    result = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    return result.stdout.strip(), result.returncode


def clean_dir(path):
    if os.path.exists(path):
        shutil.rmtree(path)


def create_file(path, content="", delay=False):
    with open(path, 'w') as f:
        f.write(content)
    if delay:
        time.sleep(1)  # Ensure mod time differs


def print_result(test_name, passed, output=None):
    print(f"[{'PASS' if passed else 'FAIL'}] {test_name}")
    if not passed and output:
        print("Output:")
        print(output)
        print('-' * 40)


def test_usage_message():
    output, code = run([EXECUTABLE])
    expected = "Usage: file_sync <source_directory> <destination_directory>"
    print_result("Incorrect args (no args)", expected in output and code == 1, output)


def test_missing_source():
    dest = 'test_dest'
    clean_dir(dest)
    output, code = run([EXECUTABLE, 'nonexistent_src', dest])
    expected = "Error: Source directory 'nonexistent_src' does not exist."
    print_result("Missing source dir", expected in output and code == 1, output)


def test_create_destination():
    src, dest = 'test_src', 'test_new_dest'
    clean_dir(src)
    clean_dir(dest)
    os.mkdir(src)
    create_file(os.path.join(src, 'a.txt'), 'Hello!')
    output, code = run([EXECUTABLE, src, dest])
    passed = "Created destination directory 'test_new_dest'." in output and "New file found: a.txt" in output
    print_result("Create destination dir", passed, output)
    clean_dir(src)
    clean_dir(dest)


def test_identical_files():
    src, dest = 'test_src', 'test_dest'
    clean_dir(src)
    clean_dir(dest)
    os.mkdir(src)
    os.mkdir(dest)
    create_file(os.path.join(src, 'a.txt'), 'same content')
    create_file(os.path.join(dest, 'a.txt'), 'same content')
    output, code = run([EXECUTABLE, src, dest])
    passed = "File a.txt is identical. Skipping..." in output
    print_result("Identical files", passed, output)
    clean_dir(src)
    clean_dir(dest)


def test_file_update():
    src, dest = 'test_src', 'test_dest'
    clean_dir(src)
    clean_dir(dest)
    os.mkdir(src)
    os.mkdir(dest)
    create_file(os.path.join(dest, 'a.txt'), 'old content')
    time.sleep(1)
    create_file(os.path.join(src, 'a.txt'), 'new content')
    output, code = run([EXECUTABLE, src, dest])
    passed = "File a.txt is newer in source. Updating..." in output
    print_result("Newer file in source", passed, output)
    clean_dir(src)
    clean_dir(dest)


def test_file_skip_update():
    src, dest = 'test_src', 'test_dest'
    clean_dir(src)
    clean_dir(dest)
    os.mkdir(src)
    os.mkdir(dest)
    create_file(os.path.join(src, 'a.txt'), 'old content')
    time.sleep(1)
    create_file(os.path.join(dest, 'a.txt'), 'new content')
    output, code = run([EXECUTABLE, src, dest])
    passed = "File a.txt is newer in destination. Skipping..." in output
    print_result("Newer file in destination", passed, output)
    clean_dir(src)
    clean_dir(dest)


def test_new_file_copy():
    src, dest = 'test_src', 'test_dest'
    clean_dir(src)
    clean_dir(dest)
    os.mkdir(src)
    os.mkdir(dest)
    create_file(os.path.join(src, 'b.txt'), 'brand new file')
    output, code = run([EXECUTABLE, src, dest])
    dest_file = os.path.join(dest, 'b.txt')
    passed = os.path.exists(dest_file) and filecmp.cmp(os.path.join(src, 'b.txt'), dest_file)
    print_result("Copy new file", passed, output)
    clean_dir(src)
    clean_dir(dest)


def test_max_file_limit():
    src, dest = 'test_src', 'test_dest'
    clean_dir(src)
    clean_dir(dest)
    os.mkdir(src)
    os.mkdir(dest)
    for i in range(100):
        create_file(os.path.join(src, f'file{i:03}.txt'), f'File {i}')
    output, code = run([EXECUTABLE, src, dest])
    passed = "Synchronization complete." in output
    print_result("Handle 100 files", passed, output)
    clean_dir(src)
    clean_dir(dest)

def test_deep_directory_creation():
    src = 'test_src'
    dest = os.path.join('/tmp', 'new1', 'new2', 'new3')
    clean_dir(src)
    clean_dir('/tmp/new1')
    os.mkdir(src)
    create_file(os.path.join(src, 'x.txt'), 'deep')
    output, code = run([EXECUTABLE, src, dest])
    expected = f"Created destination directory '{dest}'."
    passed = expected in output and os.path.exists(os.path.join(dest, 'x.txt'))
    print_result("Deep destination dir creation", passed, output)
    clean_dir(src)
    clean_dir('/tmp/new1')

def test_different_parents():
    src = '/tmp/test_src_other'
    dest = '/var/tmp/test_dest_other'
    clean_dir(src)
    clean_dir(dest)
    os.makedirs(src)
    os.makedirs(dest)
    create_file(os.path.join(src, 'unique.txt'), 'diff parent')
    output, code = run([EXECUTABLE, src, dest])
    passed = "New file found: unique.txt" in output
    print_result("Different parent folders", passed, output)
    clean_dir(src)
    clean_dir(dest)


def test_relative_vs_absolute():
    os.makedirs('rel_src', exist_ok=True)
    os.makedirs('rel_dest', exist_ok=True)
    create_file('rel_src/test.txt', 'test content')
    abs_src = os.path.abspath('rel_src')
    rel_dest = 'rel_dest'
    output, code = run([EXECUTABLE, abs_src, rel_dest])
    passed = "New file found: test.txt" in output
    print_result("Relative vs absolute path mix", passed, output)
    clean_dir('rel_src')
    clean_dir('rel_dest')


def test_empty_source_dir():
    src, dest = 'empty_src', 'empty_dest'
    clean_dir(src)
    clean_dir(dest)
    os.mkdir(src)
    os.mkdir(dest)
    output, code = run([EXECUTABLE, src, dest])
    passed = "Synchronization complete." in output and "New file found" not in output
    print_result("Empty source directory", passed, output)
    clean_dir(src)
    clean_dir(dest)


def test_files_with_spaces():
    src, dest = 'space_src', 'space_dest'
    clean_dir(src)
    clean_dir(dest)
    os.mkdir(src)
    os.mkdir(dest)
    create_file(os.path.join(src, 'file with space.txt'), 'spaced out')
    output, code = run([EXECUTABLE, src, dest])
    passed = "New file found: file with space.txt" in output
    print_result("Files with spaces", passed, output)
    clean_dir(src)
    clean_dir(dest)


def test_ignore_subdirectories():
    src, dest = 'src_with_dirs', 'dest_with_dirs'
    clean_dir(src)
    clean_dir(dest)
    os.mkdir(src)
    os.mkdir(dest)
    os.mkdir(os.path.join(src, 'subdir'))  # should be ignored
    create_file(os.path.join(src, 'realfile.txt'), 'real file')
    output, code = run([EXECUTABLE, src, dest])
    passed = "realfile.txt" in output and "subdir" not in output
    print_result("Skip subdirectories", passed, output)
    clean_dir(src)
    clean_dir(dest)

def test_alphabetical_order():
    src, dest = 'alpha_src', 'alpha_dest'
    clean_dir(src)
    clean_dir(dest)
    os.mkdir(src)
    os.mkdir(dest)

    # Create files in non-alphabetical order
    filenames = ['zeta.txt', 'alpha.txt', 'delta.txt', 'beta.txt']
    for name in filenames:
        create_file(os.path.join(src, name), f'Content of {name}')

    output, code = run([EXECUTABLE, src, dest])

    # Extract files mentioned in output
    reported = []
    for line in output.splitlines():
        if "New file found:" in line:
            fname = line.split("New file found:")[-1].strip()
            reported.append(fname)

    passed = reported == sorted(filenames)
    print_result("Alphabetical order check", passed, "\nReported order:\n" + "\n".join(reported))
    
    clean_dir(src)
    clean_dir(dest)

if __name__ == "__main__":
    EXECUTABLE = sys.argv[1] if len(sys.argv) > 1 and sys.argv[1] else EXECUTABLE

    test_usage_message()
    test_missing_source()
    test_create_destination()
    test_identical_files()
    test_file_update()
    test_file_skip_update()
    test_new_file_copy()
    test_max_file_limit()
    test_deep_directory_creation()
    test_different_parents()
    test_relative_vs_absolute()
    test_empty_source_dir()
    test_files_with_spaces()
    test_ignore_subdirectories()
    test_alphabetical_order()
