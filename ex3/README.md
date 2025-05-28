# Excercise 3: Operating Systems

## Due Date
The due date for this exercise is **12.6.2025, 23:59**.

## Submission Instructions
Submit your solution as a zip file containing **ONLY** the following files:
- `ex3.c`
- `Focus-Mode.c`
- `CPU-Scheduler.c`

The file `ex3.c` should serve as the main entry point. Based on the first command-line argument, it should execute the appropriate part (Focus-Mode or CPU-Scheduler), delegating the logic to the corresponding source file.

### Submission Link
Please submit your zip file to the submission system: [https://submit.cs.biu.ac.il/cgi-bin/welcome.cgi](https://submit.cs.biu.ac.il/cgi-bin/welcome.cgi)

## Exercise Instructions

This exercise focuses on **signals**. You may use any related material discussed in class up to and including Practical Session 8 that you find helpful.

## 🎯 WARMUP: Focus Mode Simulator (20% of the grade)

To kick things off, you'll build a simple simulation that helps you practice Unix signal handling and masking. This simulator models a scenario where a student tries to stay focused while different distractions attempt to interrupt them.
You can read more about this great time management technique [here](https://www.intelligentchange.com/blogs/read/focus-time-technique).

### 🚀 Overview

You will implement a **Focus Mode Simulator** that:

* Lets a user simulate rounds of focused work.
* Uses **Unix signals** to simulate incoming **distractions**.
* **Blocks signals** during each focus round.
* At the end of the round, **checks for pending signals**, unblocks them, and handles them one by one.
* Displays a corresponding **outcome** for each distraction.

This will help you become familiar with:

* Signal registration (`sigaction`)
* Signal blocking/unblocking (`sigprocmask`)
* Handling pending signals (`sigpending`, `sigismember`)
* Designing signal-based event logic in C

### 🔧 How it Works

Your simulator will support exactly **3 types of distractions**:

1. **Email notification** — Simulate an academic announcement
   
   Message after handling:
   `"[Outcome:] The TA announced: Everyone got 100 on the exercise!"`

3. **Reminder to pick up delivery** — Simulate a timed reminder
   
   Message after handling:
   `"[Outcome:] You picked it up just in time."`

5. **Doorbell ringing** — Simulate a food delivery
   
   Message after handling:
   `"[Outcome:] Food delivery is here."`

If the user enters `q`, the current round should end immediately, and the program should proceed to the next round without waiting for the remaining inputs.

At the end of each round, display any pending distractions and their corresponding outcomes in the specified format shown below. They should be listed in the same order as defined above (1, 2, 3), without duplicates — as we discussed in class, identical signals are not queued.

🛠️ You may **choose any 3 Unix signals** (except `SIGKILL` or `SIGSTOP`, of course) to simulate each of these distractions, just ensure to stay consistent!

### 🔌 Program Execution

As mentioned, `ex3.c` is expected to behave differently based on the **first argument** passed to it.

If the **first argument is** `"Focus-Mode"` (case-sensitive), then the **next two arguments must be provided** and must be **positive integers**:

1. **Number of Focus Rounds** — how many focus rounds to run.
2. **Duration of Each Round** — how many input cycles occur per round.

For example, to compile and run the program in **Focus Mode** for **5 rounds**, each with **3 iterations**, run:

```bash
gcc ex3.c -o ex3
./ex3 Focus-Mode 5 3
```
Here:

* `"Focus-Mode"` selects the focus simulation mode.
* `5` means 5 rounds of focus mode.
* `3` means 3 input prompts per round.

### ⚠️ Arguments Validation:

If the program is run with insufficient arguments, or if the first argument is not either Focus-Mode or CPU-Scheduler, it should display the following usage message:

```c
printf("Usage: %s <Focus-Mode/CPU-Schedule> <Num-Of-Rounds/Processes.csv> <Round-Duration/Time-Quantum>", argv[0]);
```

You **don’t need to deeply validate** the argument values (like checking if they’re numeric) — just check that:

* The argument count is correct
* The first argument is `"Focus-Mode"` or `"CPU-Scheduler"`. The rest is assumed to be correct.

**Use the provided `ex3.c`, `Focus-Mode.c`, `CPU-Scheduler.c` templates as reference if you want.**

### 🖨️ Expected Output Format

Let’s walk through the previous example: running 5 rounds with 3 iterations each.

The user input shown below is enclosed in double asterisks (**):

```text
Entering Focus Mode. All distractions are blocked.
══════════════════════════════════════════════
                Focus Round 1                
──────────────────────────────────────────────

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **1**

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **2**

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **3**
──────────────────────────────────────────────
        Checking pending distractions...
──────────────────────────────────────────────
 - Email notification is waiting.
[Outcome:] The TA announced: Everyone get 100 on the exercise!
 - You have a reminder to pick up your delivery.
[Outcome:] You picked it up just in time.
 - The doorbell is ringing.
[Outcome:] Food delivery is here.
──────────────────────────────────────────────
             Back to Focus Mode.
══════════════════════════════════════════════
══════════════════════════════════════════════
                Focus Round 2
──────────────────────────────────────────────

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **3**

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **2**

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **1**
──────────────────────────────────────────────
        Checking pending distractions...
──────────────────────────────────────────────
 - Email notification is waiting.
[Outcome:] The TA announced: Everyone get 100 on the exercise!
 - You have a reminder to pick up your delivery.
[Outcome:] You picked it up just in time.
 - The doorbell is ringing.
[Outcome:] Food delivery is here.
──────────────────────────────────────────────
             Back to Focus Mode.
══════════════════════════════════════════════
══════════════════════════════════════════════
                Focus Round 3
──────────────────────────────────────────────

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **1**

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **1**

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **1**
──────────────────────────────────────────────
        Checking pending distractions...
──────────────────────────────────────────────
 - Email notification is waiting.
[Outcome:] The TA announced: Everyone get 100 on the exercise!
──────────────────────────────────────────────
             Back to Focus Mode.
══════════════════════════════════════════════
══════════════════════════════════════════════
                Focus Round 4
──────────────────────────────────────────────

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **2**

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **q**
──────────────────────────────────────────────
        Checking pending distractions...
──────────────────────────────────────────────
 - You have a reminder to pick up your delivery.
[Outcome:] You picked it up just in time.
──────────────────────────────────────────────
             Back to Focus Mode.
══════════════════════════════════════════════
══════════════════════════════════════════════
                Focus Round 5
──────────────────────────────────────────────

Simulate a distraction:
  1 = Email notification
  2 = Reminder to pick up delivery
  3 = Doorbell Ringing
  q = Quit
>> **q**
──────────────────────────────────────────────
        Checking pending distractions...
──────────────────────────────────────────────
No distractions reached you this round.
──────────────────────────────────────────────
             Back to Focus Mode.
══════════════════════════════════════════════

Focus Mode complete. All distractions are now unblocked.
```

### 🧪 Testing and Debugging

We’ve included 3 inputs and expected output files in the "Focus-Mode-Tests" folder for your convenience — feel free to use them to test and debug your implementation.

### ⚠️ Pay Attention!

Submissions must use all of the following methods: `sigaction`, `sigprocmask`, `sigpending`, and `sigismember` in this part.
Any submission that doesn’t will receive **zero points, no exceptions**.

---

🎯 Great job staying focused!
> Now that you’ve mastered your signal skills and are ready for a real _focus round_, it’s time to level up and dive into the main part of the exercise: the **CPU Scheduler!**
> Take a deep breath, _block out all distractions_, and let’s get started!

---

## 🖥️ MAIN PART: CPU Scheduler (80% of the grade)

You'll be building a lightweight "OS kernel" that reads a list of processes from a CSV file and schedules them according to the four classic algorithms covered in class. Throughout the project, you’ll put into practice key concepts like:

* `fork()` to create child processes (**DO NOT** use threads — we'll cover those in the next exercise!)
* `kill()` and `sigaction()` or `signal()` to communicate with them
* `alarm()` and `pause()` to simulate time and control execution
* `sigprocmask()` to manage signal blocking

This is your chance to bring theory to life — simulating real CPU scheduling using the tools of the Unix world.

### 🚀 Overview

Your job is to implement a CPU scheduling engine that:

- Reads a list of processes from a CSV file.
- Simulates execution using signals and alarms.
- Displays a detailed execution timeline for each process using the FCFS, SJF, Priority (all in non-preemptive mode for simplicity) and Round Robin [scheduling algorithms](https://en.wikipedia.org/wiki/Scheduling_(computing)).

### 🔧 How it Works

* Each line in the input CSV file represents a **single process** and follows this format:

  ```csv
  Name,Description,Arrival Time,Burst Time,Priority
  ```
  
  - The fields are **comma-separated** and should appear in the specified order.
  - `Arrival Time`, `Burst Time`, and `Priority` are **integers** representing **time in seconds**.
  - `Burst Time` and `Priority` must be positive valuse.
  - The `Name` field will not exceed 50 characters, and the `Description` field will not exceed 100 characters.
  - You can assume that the file format is valid and correctly structured.
  - You may assume there will be no more than 1000 processes, and each row in the CSV file will contain no more than 256 characters.

* Your program reads these processes and **simulates execution** of them according to each scheduling algorithm.

* To simulate the execution time of each process, use the `alarm` system call with the process's burst time. This will pause the process for the specified duration, effectively mimicking actual CPU work.

* Use `alarm` similarly to simulate idle time when no processes are ready to run — representing the CPU waiting for the next arrival.

* The simulation should be **fully deterministic**, reflecting the logic of a real OS-level scheduler.

* If two or more processes have the same values for the fields used to determine their scheduling order (according to the current scheduling algorithm), their order should be determined by their arrival time. If the arrival times are also equal, you must preserve their original order as given in the input CSV file.

* At the end of each scheduling algorithm, display a final report summarizing its results.
  - For the **FCFS, SJF, and Priority** algorithms, include the **average waiting time**, formatted to two digits after the decimal point.
  - For the **Round Robin** algorithm, display the **total turnaround time** it took to schedule all processes


### 🧠 Scheduling Algorithms to Implement

1. **FCFS** – First Come, First Served
2. **SJF** – Shortest Job First
3. **Priority Scheduling** – Lower number = higher priority
4. **Round Robin** – Schedules processes in arrival order using a fixed time quantum (from CLI argument).

  Note: Implement the first three algorithms in **non-preemptive** mode for simplicity.

### 🔌 Program Execution

If the **second argument is** `"CPU-Scheduler"` (case-sensitive), then the **next two arguments must be provided** as follows:

1. **The path to the CSV file** containing the list of processes.
2. **The time quantum** (in seconds units) to be used *only* for the **Round Robin** scheduling algorithm.

For example, to compile and run your program with a CSV file and a time quantum of 2:

```bash
gcc ex3.c -o ex3
./ex3 CPU-Scheduler processes.csv 2
```

Where:

* `processes.csv` is the file in the current directory containing the process list.
* `2` is the time quantum used *only* for the Round Robin part of the simulation.

Use the same arguments validation as described in the warmup part.

### 🖨️ Expected Output Format

Let’s walk through an example using the following `processes.csv` file:

```csv
P1,System Bootloader,0,3,5
P2,Kernel Initialization,5,4,4
P3,Device Driver Loading,4,4,1
P4,Network Services Startup,4,1,2
```

Assuming you compile and run your program as before, your program should display a well-formatted output **for each scheduling algorithm** in the following style:

```text
══════════════════════════════════════════════
>> Scheduler Mode : FCFS
>> Engine Status  : Initialized
──────────────────────────────────────────────

0 → 3: P1 Running System Bootloader.
3 → 4: Idle.
4 → 8: P3 Running Device Driver Loading.
8 → 9: P4 Running Network Services Startup.
9 → 13: P2 Running Kernel Initialization.

──────────────────────────────────────────────
>> Engine Status  : Completed
>> Summary        :
   └─ Average Waiting Time : 2.00 time units
>> End of Report
══════════════════════════════════════════════

══════════════════════════════════════════════
>> Scheduler Mode : SJF
>> Engine Status  : Initialized
──────────────────────────────────────────────

0 → 3: P1 Running System Bootloader.
3 → 4: Idle.
4 → 5: P4 Running Network Services Startup.
5 → 9: P3 Running Device Driver Loading.
9 → 13: P2 Running Kernel Initialization.

──────────────────────────────────────────────
>> Engine Status  : Completed
>> Summary        :
   └─ Average Waiting Time : 1.25 time units
>> End of Report
══════════════════════════════════════════════

══════════════════════════════════════════════
>> Scheduler Mode : Priority
>> Engine Status  : Initialized
──────────────────────────────────────────────

0 → 3: P1 Running System Bootloader.
3 → 4: Idle.
4 → 8: P3 Running Device Driver Loading.
8 → 9: P4 Running Network Services Startup.
9 → 13: P2 Running Kernel Initialization.

──────────────────────────────────────────────
>> Engine Status  : Completed
>> Summary        :
   └─ Average Waiting Time : 2.00 time units
>> End of Report
══════════════════════════════════════════════

══════════════════════════════════════════════
>> Scheduler Mode : Round Robin
>> Engine Status  : Initialized
──────────────────────────────────────────────

0 → 2: P1 Running System Bootloader.
2 → 3: P1 Running System Bootloader.
3 → 4: Idle.
4 → 6: P3 Running Device Driver Loading.
6 → 7: P4 Running Network Services Startup.
7 → 9: P2 Running Kernel Initialization.
9 → 11: P3 Running Device Driver Loading.
11 → 13: P2 Running Kernel Initialization.

──────────────────────────────────────────────
>> Engine Status  : Completed
>> Summary        :
   └─ Total Turnaround Time : 13 time units

>> End of Report
══════════════════════════════════════════════
```

Here is a demonstration video showcasing how your final execution should appear, including its speed, behavior, and timing of delays:


https://github.com/user-attachments/assets/cd982ada-34f5-464a-9022-855d41734a4e



### 🧪 Testing and Debugging

We’ve included 5 sample CSV files along with their expected outputs in the "CPU-Scheduler-Tests" folder for your convenience — feel free to use them to test and debug your implementation.
Of course, these examples don’t cover every possible scenario, so be sure to test your code thoroughly before submitting.
Think creatively about edge cases and interesting or unusual behaviors your scheduler might encounter!

### ⚠️ Pay Attention!

Submissions must use at least one of the following methods: `signal`, `sigaction`, `sigprocmask`, `sigpending`, and `sigismember` in this part.
Any submission that doesn’t will receive **zero points, no exceptions**.

---

## 📌 Tips and Hints

* Take time to plan your structure, flow and approach before diving into the code — it will save you time and prevent unnecessary headaches later!
* Be mindful of synchronization and race conditions, and address them to ensure deterministic output on every execution
* Modularize your code: parsing, scheduling logic, simulation, output formatting
* Coordinate child processes `SIGSTOP`/`SIGCONT` signals to pause/resume
* Consider using a real-time signal like [`SIGRTMIN`](https://dougsland.livejournal.com/43885.html) for more reliable and precise synchronization between processes
* Prefer using `sigaction` over the simpler `signal` function for more robust and reliable signal handling
* It's recommended to use `write` rather than [`printf`](https://unix.stackexchange.com/questions/609210/why-printf-is-not-asyc-signal-safe-function?utm_source=chatgpt.com) for output operations as it's [async-signal-safe](https://docs.oracle.com/cd/E19455-01/806-5257/gen-26/index.html?utm_source=chatgpt.com)
* Measure **Waiting Time** and **Turnaround Time** carefully

## 📫 Questions?

Reach out to us in the LEMIDA [forum](https://lemida.biu.ac.il/mod/forum/view.php?id=2693823)!

💪🏻 Happy coding, and may your waiting times be minimal!
