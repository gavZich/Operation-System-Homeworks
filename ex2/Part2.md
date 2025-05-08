# File Processing System

This program processes read and write requests from a file (i.e `requests.txt`) and modifies a data file (i.e `data.txt`) accordingly. The implementation ensures that writes insert data **without overwriting existing content** and reads retrieve the correct portion of the file.

---

## Features
- **Reads specific ranges** from `data.txt` and saves the result to `read_results.txt`.
- **Writes new data at a given offset** while preserving the rest of the file.
- **Processes requests from input file** instead of user input.
- **Handles edge cases** such as invalid offsets and maintaining file integrity.
- **Ensures data persistence** with proper file operations (`lseek`, `read`, `write`).

---

## Files
### 1. First argument to your program, I'll call it the "data file"
Contains the initial data to be modified. This file starts with up to 256-bytes mix of uppercase letters, lowercase letters, and numbers.

Example:
```
ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789abcdefghijklmnopqrstuvwxyz
```

### 2. Second argument to your program, I'll call it the "requests file"
Contains a series of read (`R`), write (`W`), and quit (`Q`) commands.

#### **Format**
```
R <start_offset> <end_offset>
W <offset> <text>
Q
```
- **`R` (Read):** Reads from `data.txt` between `start_offset` and `end_offset`, inclusive.
- **`W` (Write):** Writes `<text>` at `offset` (see the examples), shifting existing data forward.
- **`Q` (Quit):** Stops processing requests.

### 3. read_results.txt
Contains the output from the read commands, with new line between every read.


## How to Run The program
```bash
omer@Omer:~/uni/os/targilim/2025B/ex2/part2$ gcc -o file_processor file_processor.c
omer@Omer:~/uni/os/targilim/2025B/ex2/part2$ ./file_processor data.txt requests.txt
```

---

## Example Execution
### **Before Running** (`data.txt`)
```
1234567890

```

### **Requests (`requests.txt`)**
```
R 2 7
W 70 THISWOULDNTSHOWUP
R 0 25
W 5 OMEROMER
W 10 ILOVEOS
R 5 20
Q
```

### **Output**
*Its not important what is the output, only the files will be checked*

### **After Running (`data.txt`)**
```
12345OMEROILOVEOSMER67890

```
### **After Running (`read_results.txt`)**
```
345678
OMEROILOVEOSMER6

```

---

## Edge Cases Handled
✅ **Reading beyond file size** → Prevent it with boundary checks, and continue to the next line (dont exit).  
✅ **Writing beyond file size** → Same as reading, **if I give you the offset of the end of the file it's ok (but after that it's not)**
    **so is offset 0 (but before that it's not).**  
✅ **Other things** → You dont need to worry about integer overflow and `start > end` in the reading.

---

## Notes
- `data.txt` and `requests.txt` must exist, otherwise `perror("data.txt")` or `perror("requests.txt")`.
- Open `read_results.txt` with O_TRUNC and O_CREAT.
- The program modifies `data.txt` permanently, so keep backups if needed.

---



