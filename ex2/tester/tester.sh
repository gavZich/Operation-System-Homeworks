#!/bin/bash

# TEST SETUP
# ...existing comments...

# ADDITIONAL TESTS
#
# TEST 4: Special Files
# 4.1 `test_4_1_empty_files`
#   This test checks if the backup program handles empty files correctly.
#
# 4.2 `test_4_2_special_chars`
#   This test checks if the backup program handles filenames with special characters ($, *, ?, etc.).
#
# 4.3 `test_4_3_permissions`
#   This test validates that the backup preserves file permissions (read-only, executable).
#
# TEST 5: Edge Cases
# 5.1 `test_5_1_long_paths`
#   This test checks handling of long file paths (approaching PATH_MAX).
#
# 5.2 `test_5_2_hidden_files`
#   This test verifies that hidden files (.dotfiles) are backed up correctly.
#
# 5.3 `test_5_3_cyclic_symlinks`
#   This test checks handling of cyclic symlinks (symlinks that point to themselves or create a cycle).

set -u

# Default values
PROGRAM=../backup     # Adjust path if needed
LOG_DIR=logs
VERBOSE=0
KEEP_DIRS=0
SPECIFIC_TEST=""
PERF_TEST=0

# Parse command line options
show_usage() {
    echo "Usage: $0 [options]"
    echo
    echo "Options:"
    echo "  -h, --help             Show this help message"
    echo "  -v, --verbose          Display more detailed output"
    echo "  -k, --keep-dirs        Don't delete test directories after test"
    echo "  -p, --program PATH     Specify path to backup program"
    echo "  -t, --test TEST_NAME   Run only specific test (e.g., 'test_1_5_files')"
    echo "  --perf                 Run performance test with large files"
    echo
}

while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_usage
            exit 0
            ;;
        -v|--verbose)
            VERBOSE=1
            shift
            ;;
        -k|--keep-dirs)
            KEEP_DIRS=1
            shift
            ;;
        -p|--program)
            PROGRAM="$2"
            shift 2
            ;;
        -t|--test)
            SPECIFIC_TEST="$2"
            shift 2
            ;;
        --perf)
            PERF_TEST=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

mkdir -p "$LOG_DIR"

pass_count=0
fail_count=0

cleanup() {
    if [ $KEEP_DIRS -eq 0 ]; then
        rm -rf "$1" "$2"
    else
        echo "Keeping test directories: $1 and $2"
    fi
}

# ... existing comparison functions ...

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
            
            # Also check permissions
            src_perms=$(stat -c %a "$src_entry")
            dst_perms=$(stat -c %a "$dst_entry")
            if [[ "$src_perms" != "$dst_perms" ]]; then
                mismatches+=("Permission mismatch for $rel_path: src=$src_perms, dst=$dst_perms")
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

    # Skip if specific test requested and this is not it
    if [[ -n "$SPECIFIC_TEST" && "$test_name" != "$SPECIFIC_TEST" ]]; then
        return
    fi

    echo "Running $test_name..."
    $setup_func "$src_dir"

    start_time=$(date +%s.%N)
    if "$PROGRAM" "$src_dir" "$dst_dir" 2>"$LOG_DIR/${test_name}_stderr.log"; then
        end_time=$(date +%s.%N)
        duration=$(echo "$end_time - $start_time" | bc)
        
        if [ $VERBOSE -eq 1 ]; then
            echo "Test execution time: $duration seconds"
        fi
        
        if compare_dirs "$src_dir" "$dst_dir"; then
            echo "✅ $test_name passed"
            cleanup "$src_dir" "$dst_dir"
            ((pass_count++))
        else
            echo "❌ $test_name failed (compare)"
            if [ $VERBOSE -eq 1 ]; then
                echo "Mismatches:"
                cat "$LOG_DIR/$test_name.log"
            fi
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
            if [ $VERBOSE -eq 1 ]; then
                echo "Program error:"
                cat "$LOG_DIR/${test_name}_stderr.log"
            fi
            ((fail_count++))
        fi
    fi
}

### Additional setup functions ###

# ... existing setup functions ...

setup_empty_files() {
    mkdir -p "$1"
    touch "$1"/empty1.txt
    touch "$1"/empty2.txt
    mkdir -p "$1"/emptydir
    touch "$1"/emptydir/empty3.txt
}

setup_special_chars() {
    mkdir -p "$1"
    touch "$1"/'file with spaces.txt'
    touch "$1"/'$special@#.txt'
    mkdir -p "$1"/'dir with spaces'
    touch "$1"/'dir with spaces'/'nested$file.txt'
}

setup_permissions() {
    mkdir -p "$1"
    echo "content" > "$1"/normal.txt
    echo "readonly" > "$1"/readonly.txt
    chmod 444 "$1"/readonly.txt  # Read-only
    echo "#!/bin/bash\necho hello" > "$1"/executable.sh
    chmod 755 "$1"/executable.sh # Executable
}

setup_long_paths() {
    mkdir -p "$1"
    local long_dir="$1"
    for i in {1..10}; do
        long_dir="$long_dir/subdir_$i"
        mkdir -p "$long_dir"
    done
    touch "$long_dir/file_at_depth_10.txt"
}

setup_hidden_files() {
    mkdir -p "$1"
    touch "$1"/.hidden1
    touch "$1"/.hidden2
    mkdir -p "$1"/.hiddendir
    touch "$1"/.hiddendir/.nestedhidden
}

setup_cyclic_symlinks() {
    mkdir -p "$1"
    touch "$1"/real_file.txt
    ln -s "$1"/loop1 "$1"/loop2  # loop2 -> loop1
    ln -s "$1"/loop2 "$1"/loop1  # loop1 -> loop2
    ln -s "$1"/self_link "$1"/self_link  # Points to itself
}

setup_large_file() {
    mkdir -p "$1"
    # Create a 100MB file
    dd if=/dev/urandom of="$1"/large_file.bin bs=1M count=100 2>/dev/null
}

### Run existing tests ###
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

### Run new tests ###
run_test test_4_1_empty_files test_4_1_src test_4_1_dst setup_empty_files

run_test test_4_2_special_chars test_4_2_src test_4_2_dst setup_special_chars

run_test test_4_3_permissions test_4_3_src test_4_3_dst setup_permissions

run_test test_5_1_long_paths test_5_1_src test_5_1_dst setup_long_paths

run_test test_5_2_hidden_files test_5_2_src test_5_2_dst setup_hidden_files

run_test test_5_3_cyclic_symlinks test_5_3_src test_5_3_dst setup_cyclic_symlinks

# Run performance test only if requested
if [ $PERF_TEST -eq 1 ]; then
    echo "Running performance test with large file..."
    run_test test_perf_large_file test_perf_src test_perf_dst setup_large_file
fi

### Summary ###
echo -e "\nSummary:"
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

    if [ $KEEP_DIRS -eq 0 ]; then
        rm -r "$LOG_DIR"
    fi
fi