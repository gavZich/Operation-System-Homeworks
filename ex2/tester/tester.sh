#!/bin/bash

# TEST SETUP
#
# This script is designed to execute a series of tests to validate the functionality of the backup program.
# The program is expected to correctly back up the content of the source directory (src) to the destination 
# directory (dst), ensuring that the backup is done in a way that preserves the directory structure, file 
# contents, symlinks, and file inodes.
#
# Each test scenario below validates a specific behavior of the backup program, and is executed with 
# dynamically created directories for a clean and isolated environment. After each test, the test log will contain 
# detailed information about any mismatches found during the backup process.
#
# Here's a breakdown of the tests and their expected behaviors:

# TEST 1: Argument Validation
#
# These tests validate how the backup program responds to different input arguments, including edge cases and invalid inputs.
#
# 1.1 `test_1_1_src_missing`
# This test simulates running the backup program with a non-existent source directory. 
# Expected behavior: The program should fail with the message "src dir: No such file or directory".
#
# 1.2 `test_1_2_dst_exist`
# This test simulates running the backup program with an already existing destination directory.
# Expected behavior: The program should fail with the message "backup dir".
#
# 1.3 `test_1_3_relative_empty`
# This test checks the backup behavior with valid relative paths for both the source and destination directories.
#
# 1.4 `test_1_4_absolute_empty`
# This test checks the backup behavior with valid absolute paths for both the source and destination directories.
#
# 1.5 `test_1_5_files`
# This test runs the backup program with a source directory containing multiple text files (a.txt, b.txt, c.txt).

# TEST 2: Directory Backup
#
# These tests validate the program's ability to handle directories and nested files correctly.
#
# 2.1 `test_2_1_nested_dirs`
# This test checks the backup behavior for nested directories (e.g., src/nested_dir/another_nested_dir).
#
# 2.2 `test_2_2_nested_files`
# This test checks the backup behavior for a mixture of files and nested directories (e.g., src/a.txt, src/nested_dir/b.txt, etc.).

# TEST 3: Symlink Handling
#
# These tests validate the program's ability to handle symbolic links correctly.
#
# 3.1 `test_3_1_symlinks`
# This test checks the backup behavior for normal symbolic links (e.g., src/symlink -> a.txt).
#
# 3.2 `test_3_2_rel_symlink`
# This test checks the backup behavior for relative symbolic links (e.g., src/my_dir/symlink -> ../a.txt).

# TEST EXECUTION
#
# Each test case is executed with a unique test setup, followed by running the backup program with the appropriate arguments.
# The test results are logged in separate log files for each test case. If any discrepancies are found during the backup, 
# a detailed list of mismatches is written to the respective log file.
#
# The log system is used to track mismatches between the source and destination directories. Each log file will record:
# - Missing directories or files: If a directory or file exists in the source but is absent in the destination.
# - Symlink mismatches: If a symlink exists in the source but its target does not match the expected target in the destination.
# - File inode mismatches: If the inodes (i.e., unique identifiers for files) do not match between the source and destination files.
#
# If the test passes successfully (no mismatches found), a success message will be printed, and the log file will not contain any errors.
# If mismatches are detected, the log file will detail the discrepancies, including:
# - Missing files or directories
# - Symlink target mismatches
# - File or directory inode mismatches
#
# The log files are stored in the directory specified by the `$LOG_DIR` variable. These log files are named after the corresponding 
# test (e.g., `test_1_1_src_missing.log`), making it easy to trace the issues for each test case. In addition, the comparison results 
# for symlinks, files, and directories are outputted for troubleshooting purposes.



set -u

PROGRAM=../backup     # Adjust path if needed
LOG_DIR=logs
mkdir -p "$LOG_DIR"

pass_count=0
fail_count=0

cleanup() {
    rm -rf "$1" "$2"
}

# Compare files by inode
compare_files() {
    local src_file=$1
    local dst_file=$2
    local src_inode dst_inode
    src_inode=$(stat -c %i "$src_file")
    dst_inode=$(stat -c %i "$dst_file")
    [[ "$src_inode" == "$dst_inode" ]]
}

# Compare symlinks by content
compare_symlinks() {
    local src_link=$1
    local dst_link=$2
    local src_target dst_target
    src_target=$(readlink "$src_link")
    dst_target=$(readlink "$dst_link")
    [[ "$src_target" == "$dst_target" ]]
}

# Recursively compare contents
compare_dirs() {
    local src=$1
    local dst=$2
    local mismatches=()
    while IFS= read -r -d '' src_entry; do
        rel_path="${src_entry#$src/}"
        dst_entry="$dst/$rel_path"

        if [ -d "$src_entry" ]; then
            [ -d "$dst_entry" ] || mismatches+=("Missing directory: $rel_path")

        elif [ -L "$src_entry" ]; then
            if [ ! -L "$dst_entry" ]; then
                mismatches+=("Missing symlink: $rel_path")
            elif ! compare_symlinks "$src_entry" "$dst_entry"; then
                mismatches+=("Symlink target mismatch: $rel_path")
            fi

        elif [ -f "$src_entry" ]; then
            if [ ! -f "$dst_entry" ]; then
                mismatches+=("Missing file: $rel_path")
            elif ! compare_files "$src_entry" "$dst_entry"; then
                mismatches+=("File inode mismatch: $rel_path")
            fi
        fi
    done < <(find "$src" -mindepth 1 -print0)

    if [ ${#mismatches[@]} -eq 0 ]; then
        return 0
    else
        printf "%s\n" "${mismatches[@]}" > "$LOG_DIR/$test_name.log"
        return 1
    fi
}

run_test() {
    test_name=$1
    src_dir=$2
    dst_dir=$3
    shift 3
    setup_func=$1
    shift

    echo "Running $test_name..."
    $setup_func "$src_dir"

    if "$PROGRAM" "$src_dir" "$dst_dir" 2>"$LOG_DIR/${test_name}_stderr.log"; then
        if compare_dirs "$src_dir" "$dst_dir"; then
            echo "✅ $test_name passed"
            cleanup "$src_dir" "$dst_dir"
            ((pass_count++))
        else
            echo "❌ $test_name failed (compare)"
            ((fail_count++))
        fi
    else
        if grep -q "src dir" "$LOG_DIR/${test_name}_stderr.log" || \
           grep -q "backup dir" "$LOG_DIR/${test_name}_stderr.log"; then
            echo "✅ $test_name passed (expected error)"
            cleanup "$src_dir" "$dst_dir"
            ((pass_count++))
        else
            echo "❌ $test_name failed (program error)"
            ((fail_count++))
        fi
    fi
}

### Setup functions ###
setup_none() {
    :
}

setup_empty_src() {
    mkdir -p "$1"
}

setup_files() {
    mkdir -p "$1"
    touch "$1"/a.txt "$1"/b.txt "$1"/c.txt
}

setup_nested_dirs() {
    mkdir -p "$1"/nested_dir/another_nested_dir
}

setup_nested_files() {
    mkdir -p "$1"/nested_dir/another_nested_dir
    echo "A" > "$1"/a.txt
    echo "B" > "$1"/nested_dir/b.txt
    echo "C" > "$1"/nested_dir/another_nested_dir/c.txt
}

setup_symlinks() {
    setup_nested_files "$1"
    touch "$1"/a.txt
    ln -s a.txt "$1"/symlink1
    touch "$1"/nested_dir/b.txt
    ln -s b.txt "$1"/nested_dir/symlink2
    touch "$1"/nested_dir/another_nested_dir/c.txt
    ln -s c.txt "$1"/nested_dir/another_nested_dir/symlink3
}

setup_rel_symlink() {
    mkdir -p "$1"/my_dir
    echo "abc" > "$1"/a.txt
    ln -s ../a.txt "$1"/my_dir/symlink
}

### Run tests ###

run_test test_1_1_src_missing "no_src_dir" test_1_1_dst setup_none

mkdir -p test_1_2_dst
run_test test_1_2_dst_exist test_1_2_src test_1_2_dst setup_empty_src

run_test test_1_3_relative_empty test_1_3_src test_1_3_dst setup_empty_src

run_test test_1_4_absolute_empty "$PWD/test_1_4_src" "$PWD/test_1_4_dst" setup_empty_src

run_test test_1_5_files test_1_5_src test_1_5_dst setup_files

run_test test_2_1_nested_dirs test_2_1_src test_2_1_dst setup_nested_dirs

run_test test_2_2_nested_files test_2_2_src test_2_2_dst setup_nested_files

run_test test_3_1_symlinks test_3_1_src test_3_1_dst setup_symlinks

run_test test_3_2_rel_symlink test_3_2_src test_3_2_dst setup_rel_symlink


### Summary ###
echo "\nSummary:"
echo "✅ Passed: $pass_count"
echo "❌ Failed: $fail_count"
if [ $fail_count -eq 0 ]; then
    echo "🎉 All tests passed!"
    cat << 'EOF'
   @..@
  (----)
 ( >__< )
 ^^ ~~ ^^
EOF


    rm -r "$LOG_DIR"

fi
