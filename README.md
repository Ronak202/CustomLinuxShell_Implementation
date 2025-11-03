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
  - **Pipelines:** Commands can be connected using `|` (e.g., `ls | grep txt`), allowing output of one command to be used as input to the next.

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
## Screenshots and Outputs

1. **Shell Startup + Basic Commands**  
<img width="739" height="392" alt="Shell Startup" src="https://github.com/user-attachments/assets/172227be-ceeb-412c-b2c8-628a31a0e24f" />

2. **Changing Directory**  
<img width="366" height="78" alt="Changing Directory" src="https://github.com/user-attachments/assets/43c10267-52e0-4065-93ed-f6bde6136798" />

3. **Sequential Commands (##)**  
<img width="757" height="97" alt="Sequential Commands" src="https://github.com/user-attachments/assets/dcd2701c-4c81-4655-8fbe-9c799fbe3359" />

4. **Parallel Commands (&&)**  
<img width="858" height="48" alt="Parallel Commands" src="https://github.com/user-attachments/assets/649d35b5-536b-4de2-9959-970fc7af88c9" />

5. **Output Redirection (>)**  
<img width="557" height="302" alt="Output Redirection" src="https://github.com/user-attachments/assets/dd63652f-d6ff-4017-affb-006b1f0b08be" />

6. **Incorrect Command**  
<img width="518" height="54" alt="Incorrect Command" src="https://github.com/user-attachments/assets/f2292ac7-32f4-4390-96b6-bff6ef465e44" />

7. **Pipeline**  
<img width="969" height="101" alt="Pipeline" src="https://github.com/user-attachments/assets/3bb54039-e56f-4081-acbe-4b0e1c51e89c" />

8. **Exit the Shell**  
<img width="641" height="70" alt="Exit Shell" src="https://github.com/user-attachments/assets/40022efa-108a-4dfe-89c2-fd427a2afe70" />

## Author

**Ronak Singh**  
*VNIT Nagpur – Computer Science & Engineering*

