"""
Driver code
"""
from pathlib import Path

from lexing.lexer import Lexer
from parsing.dsl import DSLProcessor
from parsing.generator import LL1CCodeGenerator

BASE_DIR = Path(__file__).parent

with open(BASE_DIR / "lexer.helk") as file:
    lexer_metadata = file.read()

with open(BASE_DIR / "parser.helk") as file:
    parser_metadata = file.read()


tokens = eval(lexer_metadata)
lexer_gen = Lexer(tokens, skip_whitespace=True)
lexer_gen.generate_c_lexer(BASE_DIR)

parser_gen = DSLProcessor(parser_metadata).generate_generator()
code_gen = LL1CCodeGenerator(parser_gen)
code_gen.generate_parser_code(BASE_DIR)
