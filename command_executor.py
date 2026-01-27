"""
Command Executor Module
Executes validated commands in a controlled subprocess environment.
"""

import subprocess
import os
from typing import Tuple


class CommandExecutor:
    """Executes shell commands in a controlled subprocess environment."""
    
    def __init__(self, working_directory: str = None):
        """
        Initialize the command executor.
        
        Args:
            working_directory: Directory to execute commands in (default: user's home)
        """
        if working_directory is None:
            self.working_directory = os.path.expanduser('~')
        else:
            self.working_directory = working_directory
        
        # Ensure the working directory exists
        if not os.path.exists(self.working_directory):
            os.makedirs(self.working_directory, exist_ok=True)
    
    def execute(self, command: str, timeout: int = 30) -> Tuple[bool, str, str]:
        """
        Execute a shell command.
        
        Args:
            command: The shell command to execute
            timeout: Maximum execution time in seconds (default: 30)
            
        Returns:
            Tuple of (success: bool, stdout: str, stderr: str)
            - success: True if command executed successfully
            - stdout: Standard output from the command
            - stderr: Standard error from the command
        """
        if not command or not command.strip():
            return False, "", "Empty command"
        
        try:
            # Execute the command in a subprocess
            # Note: Using shell=True is acceptable here because:
            # 1. All commands are validated by SafetyFilter before reaching this point
            # 2. We need shell features like pipes, redirects, and variable expansion
            # 3. Commands are from a controlled source (LLM or direct user input after validation)
            result = subprocess.run(
                command,
                shell=True,
                cwd=self.working_directory,
                capture_output=True,
                text=True,
                timeout=timeout,
                env=os.environ.copy()  # Use current environment
            )
            
            # Get stdout and stderr
            stdout = result.stdout
            stderr = result.stderr
            
            # Check if command was successful
            success = result.returncode == 0
            
            return success, stdout, stderr
            
        except subprocess.TimeoutExpired:
            return False, "", f"Command timed out after {timeout} seconds"
        
        except Exception as e:
            return False, "", f"Error executing command: {str(e)}"
    
    def get_working_directory(self) -> str:
        """
        Get the current working directory.
        
        Returns:
            Current working directory path
        """
        return self.working_directory
    
    def set_working_directory(self, directory: str) -> bool:
        """
        Set the working directory for command execution.
        
        Args:
            directory: Path to the new working directory
            
        Returns:
            True if directory was set successfully, False otherwise
        """
        try:
            # Expand user path if necessary
            directory = os.path.expanduser(directory)
            
            # Check if directory exists
            if not os.path.exists(directory):
                return False
            
            # Check if it's actually a directory
            if not os.path.isdir(directory):
                return False
            
            self.working_directory = directory
            return True
            
        except Exception:
            return False
    
    def execute_with_status(self, command: str, timeout: int = 30) -> str:
        """
        Execute a command and return formatted output with status.
        
        Args:
            command: The shell command to execute
            timeout: Maximum execution time in seconds
            
        Returns:
            Formatted output string including status, stdout, and stderr
        """
        success, stdout, stderr = self.execute(command, timeout)
        
        output_parts = []
        
        # Add stdout if present
        if stdout:
            output_parts.append(stdout.rstrip())
        
        # Add stderr if present
        if stderr:
            if output_parts:
                output_parts.append("")  # Empty line separator
            output_parts.append(f"Error: {stderr.rstrip()}")
        
        # If both are empty, indicate success or failure
        if not stdout and not stderr:
            if success:
                output_parts.append("Command executed successfully (no output)")
            else:
                output_parts.append("Command failed (no output)")
        
        return "\n".join(output_parts)
