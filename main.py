#!/usr/bin/env python3
"""
AI-Powered Natural Language Terminal
Main application entry point
"""

import os
import sys
from safety_filter import SafetyFilter
from llm_handler import LLMHandler
from command_executor import CommandExecutor
from session_logger import SessionLogger
from terminal_gui import TerminalGUI


class AITerminal:
    """Main application class for AI-Powered Terminal."""
    
    def __init__(self):
        """Initialize the AI Terminal application."""
        self.safety_filter = SafetyFilter()
        self.command_executor = CommandExecutor()
        self.session_logger = SessionLogger(log_to_file=True)
        
        # Initialize LLM handler (may raise exception if API key not configured)
        try:
            self.llm_handler = LLMHandler()
        except ValueError as e:
            print(f"\n{'='*80}")
            print("ERROR: OpenAI API Configuration Required")
            print('='*80)
            print(f"\n{str(e)}")
            print("\nSteps to configure:")
            print("1. Copy .env.example to .env")
            print("2. Edit .env and add your OpenAI API key")
            print("3. Get your API key from: https://platform.openai.com/api-keys")
            print(f"\n{'='*80}\n")
            sys.exit(1)
        
        # Create GUI
        self.gui = TerminalGUI(command_callback=self.process_command)
    
    def process_command(self, user_input: str):
        """
        Process a user command.
        
        Args:
            user_input: Natural language input from user
        """
        # Handle special commands
        if user_input.lower() in ['exit', 'quit']:
            self._handle_exit()
            return
        
        if user_input.lower() == 'help':
            self._display_help()
            return
        
        if user_input.lower() == 'clear':
            self.gui.clear_screen()
            return
        
        if user_input.lower() == 'history':
            self._display_history()
            return
        
        if user_input.lower() == 'summary':
            self._display_summary()
            return
        
        # Check if input starts with direct command indicator
        if user_input.startswith('!'):
            # Direct command execution (skip LLM)
            command = user_input[1:].strip()
            self.gui.display_command(f"[Direct] {command}")
            self._execute_command(user_input, command)
            return
        
        # Generate command using LLM
        self.gui.display_info("Generating command...")
        
        try:
            generated_command = self.llm_handler.generate_command(user_input)
            
            if not generated_command:
                self.gui.display_error("Failed to generate command. Please try rephrasing your request.")
                self.session_logger.log_interaction(
                    user_input, "", "", 
                    error="Command generation failed"
                )
                return
            
            # Display the generated command
            self.gui.display_command(generated_command)
            
            # Execute the command
            self._execute_command(user_input, generated_command)
            
        except Exception as e:
            error_msg = f"Error processing command: {str(e)}"
            self.gui.display_error(error_msg)
            self.session_logger.log_interaction(
                user_input, "", "", 
                error=error_msg
            )
    
    def _execute_command(self, natural_language: str, command: str):
        """
        Execute a command with safety checks.
        
        Args:
            natural_language: Original natural language input
            command: Generated or direct command to execute
        """
        # Validate command safety
        is_safe, safety_message = self.safety_filter.is_safe(command)
        
        if not is_safe:
            self.gui.display_error(safety_message)
            self.session_logger.log_interaction(
                natural_language, command, "", 
                error=safety_message, 
                blocked=True
            )
            return
        
        # Execute the command
        self.gui.display_info("Executing...")
        
        success, stdout, stderr = self.command_executor.execute(command)
        
        # Display output
        if stdout:
            self.gui.display_output(stdout + "\n", "normal")
        
        if stderr:
            self.gui.display_error(stderr)
        
        if not stdout and not stderr:
            if success:
                self.gui.display_info("Command executed successfully (no output)")
            else:
                self.gui.display_error("Command failed (no output)")
        
        # Log the interaction
        self.session_logger.log_interaction(
            natural_language, command, stdout, stderr, blocked=False
        )
        
        # Update working directory in prompt if command changed it
        if command.strip().startswith('cd '):
            current_dir = os.path.basename(self.command_executor.get_working_directory())
            if not current_dir:
                current_dir = '/'
            self.gui.update_prompt(f"~/{current_dir}" if current_dir != '/' else current_dir)
    
    def _display_help(self):
        """Display help information."""
        help_text = """
═══════════════════════════════════════════════════════════════════════════════
                                  HELP
═══════════════════════════════════════════════════════════════════════════════

NATURAL LANGUAGE COMMANDS:
  Simply type what you want to do in plain English. Examples:
  • "list all files in the current directory"
  • "show me disk space"
  • "create a folder called test"
  • "find all python files"
  • "what is my username"

DIRECT COMMANDS:
  Prefix with '!' to execute a command directly without AI translation:
  • !ls -la
  • !pwd
  • !echo "Hello"

BUILT-IN COMMANDS:
  help     - Display this help message
  clear    - Clear the terminal screen
  history  - Show command history for this session
  summary  - Display session summary
  exit     - Exit the terminal (also: quit)

FEATURES:
  • AI-powered natural language to command translation
  • Safety filtering to prevent dangerous commands
  • Session logging with timestamps
  • Copy/paste support (Ctrl+C / Ctrl+V)

SECURITY:
  For your safety, commands that could harm your system are automatically blocked.
  This includes operations like:
  • Deleting system directories
  • Shutting down the system
  • Commands requiring sudo/root privileges

═══════════════════════════════════════════════════════════════════════════════
"""
        self.gui.display_info(help_text)
    
    def _display_history(self):
        """Display recent command history."""
        recent_history = self.session_logger.get_recent_history(count=10)
        
        if not recent_history:
            self.gui.display_info("No command history yet.")
            return
        
        self.gui.display_info("\n" + "="*80)
        self.gui.display_info("RECENT COMMAND HISTORY (Last 10)")
        self.gui.display_info("="*80 + "\n")
        
        for i, entry in enumerate(recent_history, 1):
            self.gui.display_info(f"[{i}] {entry['timestamp']}")
            self.gui.display_output(f"    Input: {entry['natural_language']}\n", "normal")
            self.gui.display_command(f"    Command: {entry['generated_command']}")
            
            if entry['blocked']:
                self.gui.display_error("    Status: BLOCKED")
            else:
                self.gui.display_info("    Status: EXECUTED")
            
            self.gui.display_output("\n", "normal")
    
    def _display_summary(self):
        """Display session summary."""
        summary = self.session_logger.get_session_summary()
        self.gui.display_info("\n" + summary)
    
    def _handle_exit(self):
        """Handle application exit."""
        # Display session summary
        self.gui.display_info("\n" + "="*80)
        self.gui.display_info("CLOSING SESSION")
        self.gui.display_info("="*80 + "\n")
        
        summary = self.session_logger.get_session_summary()
        self.gui.display_info(summary)
        
        # Close session logger
        self.session_logger.close_session()
        
        self.gui.display_info("\nThank you for using AI-Powered Terminal!")
        self.gui.display_info("Goodbye!\n")
        
        # Close GUI after a short delay
        if self.gui.get_root():
            self.gui.get_root().after(2000, self.gui.get_root().quit)
    
    def run(self):
        """Run the AI Terminal application."""
        # Create and run GUI
        self.gui.create_window()
        self.gui.run()


def main():
    """Main entry point."""
    try:
        app = AITerminal()
        app.run()
    except KeyboardInterrupt:
        print("\n\nTerminal interrupted by user. Exiting...")
        sys.exit(0)
    except Exception as e:
        print(f"\n\nFatal error: {str(e)}")
        import traceback
        traceback.print_exc()
        sys.exit(1)


if __name__ == "__main__":
    main()
