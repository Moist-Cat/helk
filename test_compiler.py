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
        self.assertEqual(result.stdout.strip(), expected_output, )

        # Clean up
        Path(self.LLVM_IR_FILE).unlink(missing_ok=True)
        Path(self.OUTPUT_FILE).unlink(missing_ok=True)

    def test_arithmetic(self):
        expected = "\n".join(["14.000000", "20.000000", "5.000000"])
        self.run_test("arithmetic.hk", expected)

    def test_variables(self):
        expected = "\n".join(["8.000000", "5.000000"])
        self.run_test("variables.hk", expected)

    def test_functions(self):
        expected = "\n".join(["5.000000", "5.000000", "8.000000", "8.000000"])
        self.run_test("functions.hk", expected)

    def test_builtins(self):
        expected = "\n".join(
            [
                "42.000000",
            ]
        )
        self.run_test("builtins.hk", expected)

    def test_manual(self):
        expected = "\n".join(
            [
                "20.000000",
            ]
        )
        self.run_test("test1.hk", expected)

    def test_flat(self):
        expected = "\n".join(
            [
                "7.000000",
            ]
        )
        self.run_test("flat.hk", expected)

    def test_cond(self):
        expected = "\n".join(
            [
                "-5.000000",
            ]
        )
        self.run_test("cond.hk", expected)

    def test_unordered(self):
        expected = "\n".join(
            [
                "2.000000",
            ]
        )
        self.run_test("unordered_declaration.hk", expected)

    def test_unordered(self):
        expected = "\n".join(
            [
                "oo ee oo",
            ]
        )
        self.run_test("hello_world.hk", expected)

    def test_big_print(self):
        # for you
        expected = "\n".join(
            [
                "hello world\n20.000000",
            ]
        )
        self.run_test("big_print.hk", expected)

    def test_string_type_inference(self):
        expected = "\n".join(
            [
                "blob\nhello",
            ]
        )
        self.run_test("string_type_infer.hk", expected)

    def test_iff(self):
        expected = "\n".join(
            [
                "55.000000",
            ]
        )
        self.run_test("iff.hk", expected)

    def test_iff(self):
        expected = "\n".join(
            [
                "-3.000000",
            ]
        )
        self.run_test("iff_real.hk", expected)

    def test_nontrivial(self):
        expected = "\n".join(
            ["We made this many iterations:\n9.000000\nFinal value of b:\n5.000000"]
        )
        self.run_test("nontrivial.hk", expected)

    def test_fib(self):
        expected = "\n".join(
            ["55.000000"]
        )
        self.run_test("fib.hk", expected)


if __name__ == "__main__":
    unittest.main()
