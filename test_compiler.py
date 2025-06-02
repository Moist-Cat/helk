import re
import unittest
import os
import subprocess
from pathlib import Path


class TestCompiler(unittest.TestCase):
    COMPILER = "./build/comp"
    BUILTINS_FILE = "src/builtins.o"
    TEST_DIR = "tests"
    LLVM_IR_FILE = ".temp.ll"
    OUTPUT_FILE = ".temp"

    def run_test(self, program_file, expected_output):
        # Ensure clean state
        Path(self.LLVM_IR_FILE).unlink(missing_ok=True)
        Path(self.OUTPUT_FILE).unlink(missing_ok=True)
        
        try:
            # 1. Compile the test program
            test_path = os.path.join(self.TEST_DIR, program_file)
            with open(test_path, "r") as file:
                print(file.read())

            compile_cmd = [self.COMPILER, test_path]
            result = subprocess.run(
                compile_cmd,
                check=True,
                capture_output=True,
                text=True,
            )

            with open(self.LLVM_IR_FILE, "w") as file:
                file.write(result.stdout.strip())
            print(result.stdout.strip())

            # 2. Generate executable from LLVM IR
            link_cmd = [
                "clang",
                "-Wno-override-module",
                self.LLVM_IR_FILE,
                self.BUILTINS_FILE,
                "-o",
                self.OUTPUT_FILE,
            ]
            subprocess.run(link_cmd, check=True)

            # 3. Run the program and capture output
            result = subprocess.run(
                [f"./{self.OUTPUT_FILE}"], capture_output=True, text=True
            )

            # 4. Verify output
            self.assertEqual(
                result.stdout.strip(),
                expected_output.strip(),
            )
        finally:
            # Clean up
            Path(self.LLVM_IR_FILE).unlink(missing_ok=True)
            Path(self.OUTPUT_FILE).unlink(missing_ok=True)

    @classmethod
    def create_test_methods(cls):
        test_dir = Path(cls.TEST_DIR)
        for hk_file in test_dir.glob("**/*.hk"):
            # Find matching .out file
            out_file = hk_file.with_suffix(".out")
            if not out_file.exists():
                continue
                
            # Read expected output
            with open(out_file, 'r') as f:
                expected_output = f.read()

            # Generate test name
            rel_path = hk_file.relative_to(test_dir)
            test_name = f"test_{rel_path.with_suffix('')}"
            test_name = re.sub(r'[^a-zA-Z0-9_]', '_', str(test_name))
            
            # Create test method
            def test_method(self, program_file=str(rel_path), 
                           expected=expected_output):
                self.run_test(program_file, expected)
                
            # Add to class
            setattr(cls, test_name, test_method)

# Create test methods during class initialization
TestCompiler.create_test_methods()

if __name__ == "__main__":
    unittest.main()
