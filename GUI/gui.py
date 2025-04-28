import tkinter as tk
from tkinter import filedialog, messagebox
import subprocess
import time
import os

class CompilerGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Simple Compiler GUI")
        self.root.geometry("1000x600")

        title = tk.Label(root, text="My Compiler", bg="blue", fg="white", font=("Arial", 18))
        title.pack(fill=tk.X)

        frame = tk.Frame(root)
        frame.pack(pady=10, expand=True, fill=tk.BOTH)

        # ==== Source Code + Line Numbers ====
        source_frame = tk.Frame(frame)
        source_frame.grid(row=1, column=0, padx=10)

        self.source_label = tk.Label(frame, text="Source Code:")
        self.source_label.grid(row=0, column=0, sticky='w')

        self.line_numbers = tk.Text(source_frame, width=4, height=15, padx=5, pady=5, bg="lightgrey", state='disabled')
        self.line_numbers.pack(side=tk.LEFT, fill=tk.Y)

        self.source_text = tk.Text(source_frame, width=80, height=15)
        self.source_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.source_text.bind('<KeyRelease>', self.update_line_numbers)

        self.update_line_numbers()  # initialize

        # ==== Quadruples ====
        self.quad_label = tk.Label(frame, text="Quadruples:")
        self.quad_label.grid(row=0, column=1, sticky='w')

        self.quad_text = tk.Text(frame, width=30, height=10)
        self.quad_text.grid(row=1, column=1, padx=10)

        # ==== Symbol Table ====
        self.symbol_label = tk.Label(frame, text="Symbol Table:")
        self.symbol_label.grid(row=2, column=0, sticky='w')

        self.symbol_text = tk.Text(frame, width=80, height=8)
        self.symbol_text.grid(row=3, column=0, padx=10)

        # ==== Errors ====
        self.error_label = tk.Label(frame, text="Errors:")
        self.error_label.grid(row=2, column=1, sticky='w')

        self.error_text = tk.Text(frame, width=30, height=8)
        self.error_text.grid(row=3, column=1, padx=10)

        # ==== Buttons ====
        button_frame = tk.Frame(root)
        button_frame.pack(pady=10)

        self.load_source_button = tk.Button(button_frame, text="Load Source", command=self.load_source)
        self.load_source_button.grid(row=0, column=0, padx=10)

        self.compile_button = tk.Button(button_frame, text="Compile", command=self.compile_code)
        self.compile_button.grid(row=0, column=1, padx=10)

    def update_line_numbers(self, event=None):
        self.line_numbers.config(state='normal')
        self.line_numbers.delete('1.0', tk.END)

        num_lines = int(self.source_text.index('end-1c').split('.')[0])

        line_numbers_str = "\n".join(str(i) for i in range(1, num_lines + 1))
        self.line_numbers.insert('1.0', line_numbers_str)

        self.line_numbers.config(state='disabled')

    def load_source(self):
        filepath = filedialog.askopenfilename(filetypes=[("Text Files", "*.txt")])
        if filepath:
            with open(filepath, 'r') as f:
                code = f.read()
                self.source_text.delete("1.0", tk.END)
                self.source_text.insert(tk.END, code)

    def compile_code(self):
        try:
            # Get absolute path to 'outputs' folder
            base_path = os.path.abspath("outputs")

            # Save the source code
            source_code = self.source_text.get("1.0", tk.END).strip()
            # input_file = os.path.join(base_path, "input.txt")
            with open("input.txt", 'w') as f:
                f.write(source_code)

            # Run batch file
            subprocess.run(["run.bat"], shell=True, check=True)
            time.sleep(1)  # Wait for the batch file to finish

            # Read output files
            action_file = os.path.join(base_path, "action.txt")
            symbol_file = os.path.join(base_path, "symbol.txt")
            errors_file = os.path.join(base_path, "error.txt")

            quadruples = ""
            symbol_table = ""
            errors = ""

            if os.path.exists(action_file):
                with open(action_file, 'r') as f:
                    quadruples = f.read()

            if os.path.exists(symbol_file):
                with open(symbol_file, 'r') as f:
                    symbol_table = f.read()

            if os.path.exists(errors_file):
                with open(errors_file, 'r') as f:
                    errors = f.read()

            # Update GUI textboxes
            self.quad_text.delete("1.0", tk.END)
            self.quad_text.insert(tk.END, quadruples)

            self.symbol_text.delete("1.0", tk.END)
            self.symbol_text.insert(tk.END, symbol_table)

            self.error_text.delete("1.0", tk.END)
            self.error_text.insert(tk.END, errors)

        except Exception as e:
            messagebox.showerror("Error", f"Failed to compile: {e}")
        
if __name__ == "__main__":
    root = tk.Tk()
    app = CompilerGUI(root)
    root.mainloop()
