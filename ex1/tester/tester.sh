#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

CONFIG_FILE="tests_config.json"

if [[ ! -f "$CONFIG_FILE" ]]; then
    echo -e "${RED}Config file $CONFIG_FILE not found!${NC}"
    exit 1
fi

check_missing_files() {
    local OUTPUT_DIR=$1
    local EXPECTED_OUTPUT_DIR=$2
    MISSING_FILES=()
    for EXPECTED_FILE in "$EXPECTED_OUTPUT_DIR"/*; do
        FILE_NAME=$(basename "$EXPECTED_FILE")
        if [[ ! -f "$OUTPUT_DIR/$FILE_NAME" ]]; then
            MISSING_FILES+=("$FILE_NAME")
        fi
    done

    if [[ ${#MISSING_FILES[@]} -ne 0 ]]; then
        echo -e "${RED}Missing files: ${MISSING_FILES[*]}${NC}"
    else
        echo -e "${GREEN}All expected files are created.${NC}"
    fi
}

preprocess_file() {
    local FILE=$1
    sed -e '/^[[:space:]]*$/d' -e ':a' -e 'N' -e '$!ba' -e 's/\n/\x0/' -e 's/[[:space:]]*$//' -e 's/\x0/\n/g' "$FILE" | sed -e '$a\'
}

compare_file_contents() {
    local OUTPUT_DIR=$1
    local EXPECTED_OUTPUT_DIR=$2
    CONTENT_MISMATCH=0
    for EXPECTED_FILE in "$EXPECTED_OUTPUT_DIR"/*; do
        FILE_NAME=$(basename "$EXPECTED_FILE")
        if [[ -f "$OUTPUT_DIR/$FILE_NAME" ]]; then
            EXPECTED_CONTENT=$(preprocess_file "$EXPECTED_FILE")
            OUTPUT_CONTENT=$(preprocess_file "$OUTPUT_DIR/$FILE_NAME")
            DIFF=$(diff <(echo "$EXPECTED_CONTENT") <(echo "$OUTPUT_CONTENT"))
            if [[ -n "$DIFF" ]]; then
                CONTENT_MISMATCH=1
                FAIL_DIR="fails/test_$(basename "$OUTPUT_DIR")"
                mkdir -p "$FAIL_DIR"
                echo "$DIFF" > "$FAIL_DIR/diff_$FILE_NAME.txt"
            fi
        fi
    done

    if [[ $CONTENT_MISMATCH -eq 0 ]]; then
        echo -e "${GREEN}All file contents match the expected output.${NC}"
    fi
}

run_part_1_tests() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}          Beginning Tests for Part 1          ${NC}"
    echo -e "${CYAN}========================================${NC}"

    cd ..

    if [[ -f "split_pgn.sh" ]]; then
        SPLIT_SCRIPT="split_pgn.sh"
    elif [[ -f "pgn_split.sh" ]]; then
        SPLIT_SCRIPT="pgn_split.sh"
    else
        echo -e "${RED}No split script found (split_pgn.sh or pgn_split.sh)!${NC}"
        return
    fi

    chmod +x "$SPLIT_SCRIPT"
    cp "$SPLIT_SCRIPT" "$CURRENT_DIR"

    cd "$CURRENT_DIR"

    if [[ $SHELL == *"cbash"* ]]; then
        sed -i 's|#!/bin/bash|#!/bin/cbash|' "$SPLIT_SCRIPT"
    fi

    for TEST in $PART_1_TESTS; do
        INPUT=$(echo "$TEST" | jq -r '.input')
        OUTPUT_DIR=$(echo "$TEST" | jq -r '.output_dir')
        EXPECTED_OUTPUT_DIR=$(echo "$TEST" | jq -r '.expected_output_directory')

        echo -e "${YELLOW}----------------------------------------${NC}"
        echo -e "${YELLOW}Running test with input: ${BLUE}$INPUT${NC}"
        echo -e "${YELLOW}Output directory: ${BLUE}$OUTPUT_DIR${NC}"
        echo -e "${YELLOW}----------------------------------------${NC}"

        mkdir -p "$OUTPUT_DIR"

        echo -e "${BLUE}Running the split script...${NC}"
        if ! ./"$SPLIT_SCRIPT" "$INPUT" "$OUTPUT_DIR" > /dev/null 2>&1; then
            echo -e "${RED}Failed to run the split script${NC}"
            continue
        fi

        echo -e "${BLUE}Checking for missing files...${NC}"
        check_missing_files "$OUTPUT_DIR" "$EXPECTED_OUTPUT_DIR"

        echo -e "${BLUE}Comparing file contents...${NC}"
        compare_file_contents "$OUTPUT_DIR" "$EXPECTED_OUTPUT_DIR"

        echo -e "${BLUE}Cleaning up...${NC}"
        rm -r "$OUTPUT_DIR"

        echo -e "${YELLOW}----------------------------------------${NC}"
    done

    rm -f "$SPLIT_SCRIPT"
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}         Tests for Part 1 completed         ${NC}"
    echo -e "${CYAN}========================================${NC}\n\n"
}

run_part_2_tests() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}          Beginning Tests for Part 2          ${NC}"
    echo -e "${CYAN}========================================${NC}"

    cd ..

    if [[ -f "chess_sim.sh" ]]; then
        CHESS_SIM_SCRIPT="chess_sim.sh"
    else
        echo -e "${RED}No chess simulator script found (chess_sim.sh)!${NC}"
        return
    fi

    if [[ -f "chess_sim.py" ]]; then
        CHESS_SIM_PY="chess_sim.py"
    else
        echo -e "${RED}No chess simulator script found (chess_sim.py)!${NC}"
        return
    fi

    chmod +x "$CHESS_SIM_SCRIPT"
    cp "$CHESS_SIM_SCRIPT" "$CURRENT_DIR"
    cp "$CHESS_SIM_PY" "$CURRENT_DIR"

    cd "$CURRENT_DIR"

    if [[ $SHELL == *"cbash"* ]]; then
        sed -i 's|#!/bin/bash|#!/bin/cbash|' "$CHESS_SIM_SCRIPT"
    fi

    TEST_NUM=0

    for TEST in $PART_2_TESTS; do
        ((TEST_NUM++))
        INPUT_PATH_PGN=$(echo "$TEST" | jq -r '.input_path_pgn')
        MOVES=$(echo "$TEST" | jq -r '.moves')

        echo -e "${YELLOW}----------------------------------------${NC}"
        echo -e "${YELLOW}Running test with input: ${BLUE}$INPUT_PATH_PGN${NC}"
        echo -e "${YELLOW}Moves: ${BLUE}$MOVES${NC}"
        echo -e "${YELLOW}----------------------------------------${NC}"

        TEMP_DIR=$(mktemp -d)

        echo -e "${BLUE}Running the chess simulator script...${NC}"

        MOVES_WITH_ENTER=$(echo "$MOVES" | sed 's/./&\n/g')

        # Run chess_sim.sh
        echo -e "$MOVES_WITH_ENTER" | ./"$CHESS_SIM_SCRIPT" "$INPUT_PATH_PGN" > "$TEMP_DIR/student_output.txt" 2>&1

        # Run chess_sim.py
        echo -e "$MOVES_WITH_ENTER" | python3 "$CHESS_SIM_PY" "$INPUT_PATH_PGN" > "$TEMP_DIR/expected_output.txt" 2>&1

        # Compare outputs
        DIFF=$(diff -u "$TEMP_DIR/student_output.txt" "$TEMP_DIR/expected_output.txt")

        if [[ -z "$DIFF" ]]; then
            echo -e "${GREEN}Output matches the expected output.${NC}"
        else
            echo -e "${RED}Test $TEST_NUM failed.${NC}"
            FAIL_DIR="fails/test_$TEST_NUM"
            mkdir -p "$FAIL_DIR"
            cp "$TEMP_DIR/student_output.txt" "$FAIL_DIR/student_output.txt"
            cp "$TEMP_DIR/expected_output.txt" "$FAIL_DIR/expected_output.txt"
        fi

        echo -e "${BLUE}Cleaning up...${NC}"
        rm -r "$TEMP_DIR"

        echo -e "${YELLOW}----------------------------------------${NC}"
    done

    rm -f "$CHESS_SIM_SCRIPT"
    rm -f "$CHESS_SIM_PY"

    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}         Tests for Part 2 completed         ${NC}"
    echo -e "${CYAN}========================================${NC}\n\n"
}

run_part_2_special_tests() {
    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}      Beginning Tests for Part 2 Special     ${NC}"
    echo -e "${CYAN}========================================${NC}"

    cd ..

    if [[ -f "chess_sim.sh" ]]; then
        CHESS_SIM_SCRIPT="chess_sim.sh"
    else
        echo -e "${RED}No chess simulator script found (chess_sim.sh)!${NC}"
        return
    fi

    if [[ -f "chess_sim.py" ]]; then
        CHESS_SIM_PY="chess_sim.py"
    else
        echo -e "${RED}No chess simulator script found (chess_sim.py)!${NC}"
        return
    fi

    chmod +x "$CHESS_SIM_SCRIPT"
    cp "$CHESS_SIM_SCRIPT" "$CURRENT_DIR"
    cp "$CHESS_SIM_PY" "$CURRENT_DIR"

    cd "$CURRENT_DIR"

    if [[ $SHELL == *"cbash"* ]]; then
        sed -i 's|#!/bin/bash|#!/bin/cbash|' "$CHESS_SIM_SCRIPT"
    fi

    TEST_NUM=0
    ALL_PASSED=1

    for TEST in $PART_2_SPECIAL_TESTS; do
        ((TEST_NUM++))
        INPUT_PATH_PGN=$(echo "$TEST" | jq -r '.input_path_pgn')
        MOVES=$(echo "$TEST" | jq -r '.moves')

        echo -e "${YELLOW}----------------------------------------${NC}"
        echo -e "${YELLOW}Running test with input: ${BLUE}$INPUT_PATH_PGN${NC}"
        echo -e "${YELLOW}Moves: ${BLUE}$MOVES${NC}"
        echo -e "${YELLOW}----------------------------------------${NC}"

        TEMP_DIR=$(mktemp -d)

        echo -e "${BLUE}Running the chess simulator script...${NC}"

        MOVES_WITH_ENTER=$(echo "$MOVES" | sed 's/./&\n/g')

        # Run chess_sim.sh
        echo -e "$MOVES_WITH_ENTER" | ./"$CHESS_SIM_SCRIPT" "$INPUT_PATH_PGN" > "$TEMP_DIR/student_output.txt" 2>&1

        # Run chess_sim.py
        echo -e "$MOVES_WITH_ENTER" | python3 "$CHESS_SIM_PY" "$INPUT_PATH_PGN" > "$TEMP_DIR/expected_output.txt" 2>&1

        # Compare outputs
        DIFF=$(diff -u "$TEMP_DIR/student_output.txt" "$TEMP_DIR/expected_output.txt")

        if [[ -z "$DIFF" ]]; then
            echo -e "${GREEN}Output matches the expected output.${NC}"
        else
            echo -e "${RED}Special Test $TEST_NUM failed.${NC}"
            ALL_PASSED=0
            FAIL_DIR="fails/test_special_$TEST_NUM"
            mkdir -p "$FAIL_DIR"
            cp "$TEMP_DIR/student_output.txt" "$FAIL_DIR/student_output.txt"
            cp "$TEMP_DIR/expected_output.txt" "$FAIL_DIR/expected_output.txt"
        fi

        echo -e "${BLUE}Cleaning up...${NC}"
        rm -r "$TEMP_DIR"

        echo -e "${YELLOW}----------------------------------------${NC}"
    done

    rm -f "$CHESS_SIM_SCRIPT"
    rm -f "$CHESS_SIM_PY"

    if [[ $ALL_PASSED -eq 1 ]]; then
        echo -e "${GREEN}All special case tests passed! You will be getting 10 points bonus.${NC}"
    fi

    echo -e "${CYAN}========================================${NC}"
    echo -e "${CYAN}     Tests for Part 2 Special completed     ${NC}"
    echo -e "${CYAN}========================================${NC}\n\n"
}

CURRENT_DIR=$(pwd)

PART_1_TESTS=$(jq -c '.part_1[]' "$CONFIG_FILE")
PART_2_TESTS=$(jq -c '.part_2[]' "$CONFIG_FILE")
PART_2_SPECIAL_TESTS=$(jq -c '.part_2_special[]' "$CONFIG_FILE")

run_part_1_tests

run_part_2_tests

run_part_2_special_tests
