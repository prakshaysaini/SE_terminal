"""
Terminal GUI Module
Creates a graphical user interface that resembles a traditional terminal.
"""

import tkinter as tk
from tkinter import scrolledtext, messagebox
from typing import Callable, Optional
import threading


class TerminalGUI:
    """Terminal-style graphical user interface."""
    
    def __init__(self, command_callback: Callable[[str], None]):
        """
        Initialize the terminal GUI.
        
        Args:
            command_callback: Function to call when user submits a command
        """
        self.command_callback = command_callback
        self.root = None
        self.output_area = None
        self.input_field = None
        self.current_directory = "~"
        
        # Terminal colors (traditional terminal theme)
        self.bg_color = "#000000"  # Black background
        self.fg_color = "#00FF00"  # Green text
        self.input_bg_color = "#111111"  # Slightly lighter black
        self.error_color = "#FF0000"  # Red for errors
        self.info_color = "#00FFFF"  # Cyan for info
        self.command_color = "#FFFF00"  # Yellow for commands
    
    def create_window(self):
        """Create and configure the terminal window."""
        self.root = tk.Tk()
        self.root.title("AI-Powered Terminal")
        self.root.geometry("900x600")
        self.root.configure(bg=self.bg_color)
        
        # Set minimum window size
        self.root.minsize(600, 400)
        
        # Create header frame
        header_frame = tk.Frame(self.root, bg=self.bg_color)
        header_frame.pack(fill=tk.X, padx=5, pady=5)
        
        # Add title label
        title_label = tk.Label(
            header_frame,
            text="AI-Powered Natural Language Terminal",
            font=("Courier", 12, "bold"),
            bg=self.bg_color,
            fg=self.info_color
        )
        title_label.pack(side=tk.LEFT)
        
        # Add info label
        info_label = tk.Label(
            header_frame,
            text="Type natural language commands or 'help' for assistance",
            font=("Courier", 9),
            bg=self.bg_color,
            fg=self.fg_color
        )
        info_label.pack(side=tk.RIGHT)
        
        # Create output area (scrolled text widget)
        self.output_area = scrolledtext.ScrolledText(
            self.root,
            wrap=tk.WORD,
            font=("Courier", 10),
            bg=self.bg_color,
            fg=self.fg_color,
            insertbackground=self.fg_color,
            state=tk.DISABLED,
            height=30
        )
        self.output_area.pack(fill=tk.BOTH, expand=True, padx=5, pady=5)
        
        # Configure text tags for colors
        self.output_area.tag_config("error", foreground=self.error_color)
        self.output_area.tag_config("info", foreground=self.info_color)
        self.output_area.tag_config("command", foreground=self.command_color)
        self.output_area.tag_config("normal", foreground=self.fg_color)
        
        # Create input frame
        input_frame = tk.Frame(self.root, bg=self.bg_color)
        input_frame.pack(fill=tk.X, padx=5, pady=5)
        
        # Add prompt label
        self.prompt_label = tk.Label(
            input_frame,
            text=f"{self.current_directory} $ ",
            font=("Courier", 10, "bold"),
            bg=self.bg_color,
            fg=self.fg_color
        )
        self.prompt_label.pack(side=tk.LEFT)
        
        # Create input field
        self.input_field = tk.Entry(
            input_frame,
            font=("Courier", 10),
            bg=self.input_bg_color,
            fg=self.fg_color,
            insertbackground=self.fg_color,
            relief=tk.FLAT,
            borderwidth=2
        )
        self.input_field.pack(side=tk.LEFT, fill=tk.X, expand=True)
        self.input_field.focus()
        
        # Bind Enter key to submit command
        self.input_field.bind("<Return>", self._on_submit)
        
        # Bind Ctrl+C to copy and Ctrl+V to paste
        self.root.bind("<Control-c>", self._copy_text)
        self.root.bind("<Control-v>", self._paste_text)
        
        # Handle window close
        self.root.protocol("WM_DELETE_WINDOW", self._on_close)
        
        # Display welcome message
        self._display_welcome()
    
    def _display_welcome(self):
        """Display welcome message in the terminal."""
        welcome_text = """
╔══════════════════════════════════════════════════════════════════════════════╗
║                   AI-Powered Natural Language Terminal                       ║
║                           Version 1.0                                         ║
╚══════════════════════════════════════════════════════════════════════════════╝

Welcome! This terminal converts your natural language instructions into Linux commands.

Usage:
  • Type your instruction in plain English (e.g., "list all files")
  • The AI will generate and execute the corresponding command
  • Type 'help' for more information
  • Type 'exit' or 'quit' to close the terminal

Examples:
  • "show me all python files in this directory"
  • "create a folder named test_data"
  • "what is my current directory"
  • "display disk usage"

Note: For security, dangerous commands are automatically blocked.

"""
        self.display_output(welcome_text, "info")
    
    def _on_submit(self, event=None):
        """Handle command submission."""
        user_input = self.input_field.get().strip()
        
        if not user_input:
            return
        
        # Clear input field
        self.input_field.delete(0, tk.END)
        
        # Display user input
        self.display_output(f"{self.current_directory} $ {user_input}\n", "normal")
        
        # Process command in a separate thread to keep GUI responsive
        threading.Thread(target=self.command_callback, args=(user_input,), daemon=True).start()
    
    def display_output(self, text: str, tag: str = "normal"):
        """
        Display text in the output area.
        
        Args:
            text: Text to display
            tag: Tag for text formatting (normal, error, info, command)
        """
        if self.output_area is None:
            return
        
        self.output_area.configure(state=tk.NORMAL)
        self.output_area.insert(tk.END, text, tag)
        self.output_area.configure(state=tk.DISABLED)
        self.output_area.see(tk.END)  # Auto-scroll to bottom
    
    def display_command(self, command: str):
        """Display generated command."""
        self.display_output(f"Generated command: {command}\n", "command")
    
    def display_error(self, error: str):
        """Display error message."""
        self.display_output(f"Error: {error}\n", "error")
    
    def display_info(self, info: str):
        """Display info message."""
        self.display_output(f"{info}\n", "info")
    
    def update_prompt(self, directory: str):
        """
        Update the command prompt with current directory.
        
        Args:
            directory: Current directory path
        """
        self.current_directory = directory
        if self.prompt_label:
            self.prompt_label.config(text=f"{directory} $ ")
    
    def clear_screen(self):
        """Clear the output area."""
        if self.output_area:
            self.output_area.configure(state=tk.NORMAL)
            self.output_area.delete(1.0, tk.END)
            self.output_area.configure(state=tk.DISABLED)
    
    def _copy_text(self, event=None):
        """Copy selected text to clipboard."""
        try:
            if self.output_area:
                selected_text = self.output_area.get(tk.SEL_FIRST, tk.SEL_LAST)
                self.root.clipboard_clear()
                self.root.clipboard_append(selected_text)
        except tk.TclError:
            pass  # No text selected
    
    def _paste_text(self, event=None):
        """Paste text from clipboard to input field."""
        try:
            clipboard_text = self.root.clipboard_get()
            self.input_field.insert(tk.INSERT, clipboard_text)
        except tk.TclError:
            pass  # Clipboard empty or unavailable
    
    def _on_close(self):
        """Handle window close event."""
        if messagebox.askokcancel("Quit", "Do you want to quit the AI Terminal?"):
            self.root.quit()
            self.root.destroy()
    
    def run(self):
        """Start the GUI event loop."""
        if self.root:
            self.root.mainloop()
    
    def get_root(self):
        """Get the root window object."""
        return self.root
