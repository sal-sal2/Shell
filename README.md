# MiniShell

MiniShell is a fully functional Unix shell implementation that provides an interactive command-line environment with support for both built-in commands and external program execution. This project handles all the fundamentals of a shell such as process management, I/O redirection, pipelines, and a command-line interface design.


##  Features

### Core Shell Functionality
- Interactive command-line interface with custom prompt
- Builtin Commands
- Command parsing with quote handling
- Command execution
- Piping
- I/O Redirection
- Command History
- Autocompletion

### Built-in Commands
```bash
$ pwd                   # Print working directory
$ cd                    # Change directory (supports ~ for home)
$ echo                  # Print arguments to stdout
$ type                  # Check if command is builtin or external
$ history               # Show command history
$ exit                  # Exit shell (saves history)
```

### Advanced Features
```bash
# Pipelines
$ ls -l | grep .txt | wc -l

# I/O Redirection
$ echo "Hello" > output.txt          # Overwrite
$ echo "World" >> output.txt         # Append
$ cat nonexistent 2> errors.txt      # Redirect stderr
$ command > out.txt 2> err.txt       # Separate streams

# Command History
$ history -r ~/.bash_history         # Read history from file
$ history -w saved_history           # Write history to file
$ history -a ~/.bash_history         # Append new commands

# Autocomplete
$ ec<TAB>              # Completes to 'echo'
$ /usr/bin/gc<TAB>     # Shows: gcc, gcov, etc.
```


## Architecture

### Project Structure
```
myshell/
├── include/            
│   ├── config.h         
│   ├── shell.h          
│   ├── builtins/
│   │   └── builtin.h    
│   ├── parser/
│   │   └── parser.h     
│   ├── executor/
│   │   └── executor.h   
│   ├── path/
│   │   └── pathutils.h  
│   └── completion/
│       └── completion.h 
│
├── src/                 
│   ├── main.c           # Entry point
│   ├── shell.c          # REPL loop
│   ├── builtins/        # Built-in commands (6 builtin commands)
│   ├── parser/          # Tokenization
│   ├── executor/        # Execution
│   ├── path/            # PATH utilities
│   └── completion/      # Autocomplete
│
├── Makefile             # Build system
└── README.md            # This file
```


##  Getting Started

### Prerequisites

```bash
# Ubuntu
sudo apt-get install build-essential libreadline-dev
```

### Installation

```bash
# Clone the repository
git clone https://github.com/sal-sal2/MiniShell.git

# Build the shell
make

# Run
./myshell
```

### Usage

```bash
# Start the shell
$ ./myshell
$ pwd
/home/user/myshell

$ echo "Hello, World!"
Hello, World!

$ cd /tmp
$ pwd
/tmp

$ ls -l | grep .txt
-rw-r--r-- 1 user user 1234 Jan 01 12:00 example.txt

$ history 5
    1  pwd
    2  echo "Hello, World!"
    3  cd /tmp
    4  pwd
    5  ls -l | grep .txt

$ exit
```

##  Development

### Build Commands

```bash
make              # Build the shell
make clean        # Remove build artifacts
make rebuild      # Clean + build
make info         # Show build configuration
make run          # Build + run
```
