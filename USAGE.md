# AI-Powered Natural Language Linux Terminal

## Overview

This is an AI-powered terminal application that allows users to interact with Linux using natural language instead of traditional command-line syntax. The application uses Large Language Models (LLMs) to translate natural language instructions into executable Linux commands, validates them for safety, and executes them in a secure environment.

## Features

- 🤖 **Natural Language Processing**: Type commands in plain English
- 🛡️ **Safety Filtering**: Automatic blocking of dangerous commands
- 🎨 **Terminal-Style GUI**: Familiar terminal interface with modern conveniences
- 📝 **Session Logging**: Complete history of all interactions
- ⚡ **Real-time Execution**: Immediate command execution with output display
- 🔒 **Secure Execution**: Commands run with user-level permissions only

## Requirements

- Linux-based operating system (Ubuntu recommended)
- Python 3.7 or higher
- Internet connection for LLM API communication
- OpenAI API key

## Installation

### 1. Clone the Repository

```bash
git clone https://github.com/prakshaysaini/SE_terminal.git
cd SE_terminal
```

### 2. Install Dependencies

```bash
pip install -r requirements.txt
```

Or install manually:
```bash
pip install openai>=1.0.0 python-dotenv>=1.0.0
```

### 3. Configure API Key

1. Copy the example environment file:
```bash
cp .env.example .env
```

2. Edit `.env` and add your OpenAI API key:
```bash
nano .env
```

Replace `your_api_key_here` with your actual OpenAI API key:
```
OPENAI_API_KEY=sk-your-actual-api-key-here
```

3. Get your API key from [OpenAI Platform](https://platform.openai.com/api-keys)

### 4. Run the Application

```bash
python3 main.py
```

Or if you made it executable:
```bash
./main.py
```

## Usage

### Natural Language Commands

Simply type what you want to do in plain English:

```
show me all files in the current directory
create a folder called test_data
what is my current directory
find all python files
display disk usage
show me system information
```

### Direct Commands

Prefix commands with `!` to execute them directly without AI translation:

```
!ls -la
!pwd
!echo "Hello World"
```

### Built-in Commands

- `help` - Display help information
- `clear` - Clear the terminal screen
- `history` - Show recent command history
- `summary` - Display session statistics
- `exit` or `quit` - Close the terminal

### Examples

```
# Natural language examples
User: list all files
Generated: ls -la

User: show disk space
Generated: df -h

User: create a directory named projects
Generated: mkdir projects

User: find all text files
Generated: find . -name "*.txt"

User: what is my username
Generated: whoami

User: show running processes
Generated: ps aux
```

## Architecture

The application consists of several modular components:

### 1. Terminal GUI (`terminal_gui.py`)
- Creates a terminal-style graphical interface using tkinter
- Handles user input and displays output
- Provides a familiar terminal experience

### 2. LLM Handler (`llm_handler.py`)
- Communicates with OpenAI API
- Translates natural language to Linux commands
- Uses GPT-3.5-turbo by default (configurable)

### 3. Safety Filter (`safety_filter.py`)
- Validates commands before execution
- Blocks dangerous operations (e.g., system deletion, shutdown)
- Prevents commands requiring elevated privileges

### 4. Command Executor (`command_executor.py`)
- Executes validated commands using Python's subprocess module
- Captures stdout and stderr
- Enforces timeout limits

### 5. Session Logger (`session_logger.py`)
- Maintains history of all interactions
- Logs to timestamped files
- Provides session statistics

## Security Features

The application includes multiple layers of security:

### Blocked Operations
- System directory deletion (`rm -rf /`, etc.)
- Filesystem formatting (`mkfs`)
- System shutdown/reboot commands
- Commands requiring sudo/root privileges
- Fork bombs and malicious patterns
- Critical system directory modifications

### Safe Commands
The following types of commands are considered safe:
- File listing and navigation (`ls`, `cd`, `pwd`)
- File content viewing (`cat`, `less`, `head`, `tail`)
- System information (`uname`, `df`, `free`, `whoami`)
- Search operations (`find`, `grep`, `locate`)
- Process viewing (`ps`, `top`)

## Configuration

### Environment Variables

Edit `.env` file to customize:

```bash
# Required: Your OpenAI API key
OPENAI_API_KEY=sk-your-key-here

# Optional: Change the AI model (default: gpt-3.5-turbo)
OPENAI_MODEL=gpt-4
```

### Available Models
- `gpt-3.5-turbo` (default, faster and cheaper)
- `gpt-4` (more accurate but slower and more expensive)

## Session Logs

Session logs are automatically saved to files with timestamps:
```
session_YYYYMMDD_HHMMSS.log
```

Each log includes:
- Timestamp of each interaction
- User's natural language input
- Generated command
- Execution status (executed or blocked)
- Command output and errors

## Performance

- **Command Generation**: Typically completes within 1-3 seconds
- **Command Execution**: Immediate, with 30-second timeout
- **Resource Usage**: Minimal CPU and memory footprint
- **Responsiveness**: GUI remains responsive during operations

## Troubleshooting

### API Key Not Found
```
ERROR: OpenAI API key not found
```
**Solution**: Create a `.env` file and add your API key

### Import Errors
```
ModuleNotFoundError: No module named 'openai'
```
**Solution**: Install dependencies with `pip install -r requirements.txt`

### Permission Denied
```
Error: Permission denied
```
**Solution**: Make sure you're not trying to execute commands that require sudo

### Command Timeout
```
Command timed out after 30 seconds
```
**Solution**: The command took too long. Try a simpler operation.

## Limitations

- Requires internet connection for AI command generation
- Commands are limited to single-line operations
- No interactive command support (commands requiring user input during execution)
- Limited to user-level permissions (no sudo/root access)
- Depends on OpenAI API availability

## Future Enhancements

- Offline AI model integration
- Multi-command sequences and pipelines
- Interactive command support
- Command history search and replay
- Customizable terminal themes
- Support for other LLM providers
- Command autocompletion
- Syntax highlighting

## Contributing

This project was developed as part of a Software Engineering course at IIIT Guwahati.

## License

See LICENSE file for details.

## Authors

- Rishabh Kumar Jain (2301175)
- Rahul Yadav (2301168)
- Prakshay Saini (2301149)

**Department**: Computer Science and Engineering  
**Institute**: Indian Institute of Information Technology, Guwahati  
**Academic Year**: 2025–2026

## Acknowledgments

- OpenAI for providing the GPT API
- Python community for excellent libraries
- IIIT Guwahati for academic support

## Support

For issues, questions, or suggestions, please open an issue on GitHub.

---

**Note**: This application is designed for educational purposes and demonstrates the integration of AI with system-level operations. Always review generated commands before execution in production environments.
