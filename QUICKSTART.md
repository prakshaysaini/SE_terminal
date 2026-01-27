# Quick Start Guide

## Installation (5 minutes)

### Prerequisites
- Linux OS (Ubuntu 20.04+ recommended)
- Python 3.7+
- pip package manager

### Step 1: Install System Dependencies (Ubuntu/Debian)

```bash
# Update package list
sudo apt update

# Install Python3 and tkinter
sudo apt install -y python3 python3-pip python3-tk
```

For other distributions:
- **Fedora/RHEL**: `sudo dnf install python3 python3-pip python3-tkinter`
- **Arch Linux**: `sudo pacman -S python python-pip tk`

### Step 2: Clone Repository

```bash
git clone https://github.com/prakshaysaini/SE_terminal.git
cd SE_terminal
```

### Step 3: Install Python Dependencies

```bash
pip3 install -r requirements.txt
```

Or install manually:
```bash
pip3 install openai>=1.0.0 python-dotenv>=1.0.0
```

### Step 4: Configure OpenAI API Key

1. Get your API key from [OpenAI Platform](https://platform.openai.com/api-keys)

2. Create `.env` file:
```bash
cp .env.example .env
nano .env
```

3. Add your API key:
```
OPENAI_API_KEY=sk-your-actual-api-key-here
```

Save and exit (Ctrl+X, then Y, then Enter)

### Step 5: Run the Application

```bash
python3 main.py
```

## First Commands to Try

Once the terminal opens, try these:

```
list all files
show disk space
what is my current directory
find all python files
create a folder named test
show system information
who am i
```

## Testing Without API Key

You can test the core components without an API key:

```bash
python3 test_components.py
```

This validates:
- ✓ Safety Filter (blocking dangerous commands)
- ✓ Command Executor (running safe commands)
- ✓ Session Logger (recording history)
- ✓ LLM Handler (initialization)

## Troubleshooting

### "No module named 'tkinter'"
```bash
sudo apt install python3-tk
```

### "OpenAI API key not found"
Make sure `.env` file exists and contains your API key.

### Permission Issues
Make sure scripts are executable:
```bash
chmod +x main.py test_components.py
```

## Usage Tips

1. **Natural Language**: Type commands in plain English
   - "show me all text files"
   - "create a backup directory"

2. **Direct Commands**: Prefix with `!` for direct execution
   - `!ls -la`
   - `!pwd`

3. **Built-in Commands**:
   - `help` - Show help
   - `clear` - Clear screen
   - `history` - Show command history
   - `exit` - Quit

## Security Notes

These commands are automatically blocked:
- System file deletion (rm -rf /)
- Shutdown/reboot commands
- Commands requiring sudo
- Dangerous disk operations
- Fork bombs

## Next Steps

See [USAGE.md](USAGE.md) for complete documentation.

## Support

For issues: https://github.com/prakshaysaini/SE_terminal/issues
