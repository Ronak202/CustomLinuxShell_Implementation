# Custom Linux Shell Implementation in C


## Overview
This project implements a basic Linux shell in **C** that can execute built-in commands as well as system commands. It supports multiple command execution, output redirection, signal handling, and directory navigation, mimicking basic bash shell functionality.

---

## Features
- **Interactive Shell**
  - Infinite loop until `exit` command.
  - Displays the current working directory followed by `$`.

- **Built-in Commands**
  - `cd <directory>`: Change the working directory.
  - `cd ..`: Move to parent directory.

- **Command Execution**
  - Parallel execution: Commands separated by `&&`.
  - Sequential execution: Commands separated by `##`.

- **Output Redirection**
  - Redirect standard output using `>` (e.g., `ls > output.txt`).

- **Signal Handling**
  - Handles `Ctrl+C` and `Ctrl+Z` gracefully without terminating the shell.

- **Optional: Command Pipelines (Extra Credit)**
  - Supports `|` to connect output of one command to the input of another.

- **Error Handling**
  - Invalid commands display: `Shell: Incorrect command`.

---

## Implementation Details
- **Language:** C
- **System Calls Used:** `fork()`, `exec()`, `wait()`, `chdir()`
- **Input Handling:** `getline()`, `strsep()`
- Modular and commented code for readability and maintainability.

---

## How to Run
1. Clone the repository:
   ```bash
   git clone https://github.com/<your-username>/custom-linux-shell.git
   cd custom-linux-shell
   
