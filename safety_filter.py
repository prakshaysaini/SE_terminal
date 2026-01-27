"""
Safety Filter Module
Validates commands before execution to prevent dangerous operations.
"""

import re
from typing import Tuple


class SafetyFilter:
    """Validates commands to prevent execution of dangerous operations."""
    
    # List of dangerous commands and patterns
    DANGEROUS_COMMANDS = [
        'rm -rf /',
        'rm -rf /*',
        'mkfs',
        'dd if=',
        '> /dev/sda',
        'mv /home',
        'chmod -R 777 /',
        ':(){:|:&};:',  # Fork bomb
    ]
    
    # Patterns that indicate dangerous operations
    DANGEROUS_PATTERNS = [
        r'rm\s+-rf\s+/',  # Recursive force deletion from root
        r'rm\s+-rf\s+/\w+',  # Deletion of root-level directories
        r'rm\s+-rf\s+\*',  # Recursive deletion with wildcard
        r'mkfs\.',  # Filesystem formatting
        r'dd\s+if=',  # Disk duplication (potentially dangerous)
        r'>\s*/dev/sd[a-z]',  # Writing to disk devices
        r'chmod\s+-R\s+777\s+/',  # Changing permissions of root
        r'shutdown',  # System shutdown
        r'reboot',  # System reboot
        r'init\s+0',  # Shutdown via init
        r'init\s+6',  # Reboot via init
        r'halt',  # System halt
        r'poweroff',  # System power off
        r'chown\s+-R',  # Recursive ownership change (potentially dangerous)
        r'mv\s+/home',  # Moving critical directories
        r'mv\s+/etc',
        r'mv\s+/usr',
        r'mv\s+/var',
        r'mv\s+/bin',
        r'mv\s+/sbin',
        r'rm\s+/etc/',  # Removing system configuration files
        r'rm\s+-rf\s+~/',  # Recursive deletion of home directory
        r':\(\)\{.*\}',  # Fork bomb pattern
    ]
    
    # Commands that should be allowed (read-only operations)
    SAFE_COMMANDS = [
        'ls', 'pwd', 'cat', 'echo', 'date', 'whoami', 'id', 'uname',
        'df', 'du', 'free', 'uptime', 'w', 'who', 'which', 'whereis',
        'head', 'tail', 'less', 'more', 'grep', 'find', 'locate',
        'ps', 'top', 'htop', 'history', 'env', 'printenv',
        'wc', 'sort', 'uniq', 'diff', 'file', 'stat',
    ]
    
    def __init__(self):
        """Initialize the safety filter."""
        pass
    
    def is_safe(self, command: str) -> Tuple[bool, str]:
        """
        Check if a command is safe to execute.
        
        Args:
            command: The command to validate
            
        Returns:
            Tuple of (is_safe: bool, message: str)
            - is_safe: True if command is safe, False otherwise
            - message: Explanation of the safety check result
        """
        if not command or not command.strip():
            return False, "Empty command"
        
        command = command.strip()
        
        # Check against dangerous command list
        for dangerous_cmd in self.DANGEROUS_COMMANDS:
            if dangerous_cmd in command:
                return False, f"Blocked: Command contains dangerous operation '{dangerous_cmd}'"
        
        # Check against dangerous patterns
        for pattern in self.DANGEROUS_PATTERNS:
            if re.search(pattern, command, re.IGNORECASE):
                return False, f"Blocked: Command matches dangerous pattern"
        
        # Additional checks for commands targeting critical system directories
        critical_dirs = ['/bin', '/sbin', '/usr', '/lib', '/lib64', '/boot', '/sys', '/proc']
        for dir_path in critical_dirs:
            # Check for deletion or modification of critical directories
            if re.search(rf'rm\s+.*{re.escape(dir_path)}', command):
                return False, f"Blocked: Attempting to remove critical system directory {dir_path}"
            if re.search(rf'mv\s+{re.escape(dir_path)}', command):
                return False, f"Blocked: Attempting to move critical system directory {dir_path}"
        
        # Check for sudo usage (requires elevated privileges)
        if command.startswith('sudo ') or ' sudo ' in command:
            return False, "Blocked: sudo commands require manual execution for security"
        
        # Check for su usage
        if command.startswith('su ') or ' su ' in command:
            return False, "Blocked: su commands require manual execution for security"
        
        # If we've made it here, the command appears safe
        return True, "Command passed safety checks"
    
    def sanitize_command(self, command: str) -> str:
        """
        Sanitize a command by removing potentially dangerous elements.
        
        Args:
            command: The command to sanitize
            
        Returns:
            Sanitized command string
        """
        # Remove leading/trailing whitespace
        command = command.strip()
        
        # Remove any semicolons or command chaining that could be used to inject commands
        # Only allow single commands for now
        if ';' in command or '&&' in command or '||' in command:
            # Take only the first command
            command = re.split(r'[;&|]+', command)[0].strip()
        
        return command
