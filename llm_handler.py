"""
LLM Handler Module
Handles communication with OpenAI API for command generation.
"""

import os
import re
from typing import Optional
from openai import OpenAI
from dotenv import load_dotenv


class LLMHandler:
    """Handles LLM API communication for natural language to command translation."""
    
    def __init__(self):
        """Initialize the LLM handler with API credentials."""
        # Load environment variables
        load_dotenv()
        
        # Get API key from environment
        api_key = os.getenv('OPENAI_API_KEY')
        if not api_key or api_key == 'your_api_key_here':
            raise ValueError(
                "OpenAI API key not found. Please set OPENAI_API_KEY in .env file.\n"
                "Copy .env.example to .env and add your API key."
            )
        
        # Initialize OpenAI client
        self.client = OpenAI(api_key=api_key)
        
        # Get model from environment or use default
        self.model = os.getenv('OPENAI_MODEL', 'gpt-3.5-turbo')
        
        # System prompt for the LLM
        self.system_prompt = """You are a Linux command translator. Convert natural language instructions into valid Linux shell commands.

Rules:
1. Respond ONLY with the Linux command, nothing else
2. Do not include explanations, backticks, or formatting
3. Generate single-line commands only
4. Use standard Linux utilities available on most distributions
5. Prioritize safe, read-only commands when possible
6. If the user asks to create/modify files, use appropriate commands like mkdir, touch, echo, etc.
7. For file operations, use the current directory unless specified otherwise
8. Do not generate commands that require sudo or root privileges
9. Keep commands simple and clear

Examples:
User: "list all files in current directory"
Response: ls -la

User: "show me disk space"
Response: df -h

User: "create a folder called test"
Response: mkdir test

User: "find all python files"
Response: find . -name "*.py"
"""
    
    def generate_command(self, natural_language: str, timeout: int = 30) -> Optional[str]:
        """
        Convert natural language input to a Linux command.
        
        Args:
            natural_language: User's natural language instruction
            timeout: API timeout in seconds (default: 30)
            
        Returns:
            Generated Linux command string, or None if generation fails
        """
        if not natural_language or not natural_language.strip():
            return None
        
        try:
            # Call OpenAI API
            response = self.client.chat.completions.create(
                model=self.model,
                messages=[
                    {"role": "system", "content": self.system_prompt},
                    {"role": "user", "content": natural_language}
                ],
                max_tokens=200,
                temperature=0.3,  # Lower temperature for more consistent outputs
                timeout=timeout
            )
            
            # Extract the command from response
            command = response.choices[0].message.content.strip()
            
            # Clean up the command (remove any markdown formatting if present)
            command = self._clean_command(command)
            
            return command
            
        except Exception as e:
            print(f"Error generating command: {str(e)}")
            return None
    
    def _clean_command(self, command: str) -> str:
        """
        Clean up the generated command by removing formatting.
        
        Args:
            command: Raw command from LLM
            
        Returns:
            Cleaned command string
        """
        # Remove markdown code blocks if present
        command = re.sub(r'^```(?:bash|sh|shell)?\n?', '', command)
        command = re.sub(r'\n?```$', '', command)
        
        # Remove backticks if present
        command = command.replace('`', '')
        
        # Remove leading/trailing whitespace
        command = command.strip()
        
        # If there are multiple lines, take only the first non-empty line
        lines = [line.strip() for line in command.split('\n') if line.strip()]
        if lines:
            command = lines[0]
        
        return command
    
    def is_available(self) -> bool:
        """
        Check if the LLM API is available and configured.
        
        Returns:
            True if API is available, False otherwise
        """
        try:
            api_key = os.getenv('OPENAI_API_KEY')
            return api_key is not None and api_key != 'your_api_key_here'
        except Exception:
            return False
