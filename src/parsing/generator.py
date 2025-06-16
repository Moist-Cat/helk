from pathlib import Path
from collections import defaultdict

from parsing.parser import LL1ParserGenerator
from parsing.dsl import DSLProcessor

BASE_DIR = Path(__file__).parent

with open(BASE_DIR / "parser_helpers.txt") as file:
    parser_helpers = file.read()

with open(BASE_DIR / "parser_main.txt") as file:
    parser_main = file.read()

with open(BASE_DIR / "parser_header.txt") as file:
    parser_header = file.read()

with open(BASE_DIR / "parser_source_header.txt") as file:
    parser_source_header = file.read()


class LL1CCodeGenerator:
    """Generates optimized C code for LL(1) parser based on parsing table."""

    def __init__(self, parser_generator):
        """
        Args:
            parser_generator: LL1ParserGenerator instance with computed parsing table
        """
        self.parser = parser_generator
        self.table = parser_generator.build_parsing_table()
        self.first = parser_generator.compute_first()
        self.follow = parser_generator.compute_follow(self.first)
        self.non_terminals = parser_generator.non_terminals
        self.terminals = parser_generator.terminals
        self.epsilon = parser_generator.epsilon
        self.code = parser_generator.code
        self.end_marker = parser_generator.end_marker
        self.errors = []

        self.ast_name = "ASTNode"

        # Map non-terminals to valid C function names
        # XXX delet this shit
        self.nt_to_func = {
            nt: f"{nt.replace('-', '_').replace('+', 'plus')}"
            for nt in self.non_terminals
        }

    def generate_parser_code(self, out_dir):
        out_dir = Path(out_dir)
        """Generate complete C parser implementation."""
        with open(out_dir / "parser.h", "w") as h:
            self._generate_header(h)

        with open(out_dir / "parser.c", "w") as c:
            self._generate_source(c)

    def _generate_header(self, f):
        """Generate header file with declarations."""

        f.write(parser_header.format(ast_name=self.ast_name))

    def _generate_source(self, f):
        """Generate source file with parser implementation."""
        f.write(parser_source_header)

        # Function prototypes for non-terminals
        for nt in self.non_terminals:
            f.write(f"{self.ast_name}* {self.nt_to_func[nt]}(void);\n")
        f.write("\n")

        # Helper functions
        f.write(parser_helpers)

        # Non-terminal functions
        for nt in self.non_terminals:
            self._generate_non_terminal_function(f, nt)

        # Main parsing function
        self._generate_main_parser(f)

    def _generate_non_terminal_function(self, f, nt):
        """Generate parsing function for a non-terminal."""
        func_name = self.nt_to_func[nt]
        # hard-coded node name (opinionated)
        f.write(f"{self.ast_name}* {func_name}(void) {{\n")
        f.write("    TokenType sync_set[] = {")

        # Generate synchronization set (follow set)
        follow_list = sorted(self.follow.get(nt, set()))
        tokens = [f"TOKEN_{t.upper()}" for t in follow_list if t != self.epsilon]
        tokens.append(f"TOKEN_{self.end_marker.upper()}")
        # set to avoid duplicates
        f.write(", ".join(set(tokens)))
        f.write("};\n")
        f.write(f"    int sync_size = sizeof(sync_set)/sizeof(sync_set[0]);\n\n")
        f.write(f"    {self.ast_name}* node = NULL;\n\n")
        f.write(
            f'    fprintf(stderr, "DEBUG - At {func_name} [current=%d]\\n", current_tok);\n\n'
        )
        # define variables
        defined = set()
        for (nt_key, token), production in self.table.items():
            if nt_key == nt:
                for prod in production:
                    if (prod == self.epsilon) or prod in defined:
                        continue
                    cls = ""
                    if prod not in self.non_terminals:
                        cls = "Token"
                    else:
                        cls = "ASTNode*"
                    f.write(f"    {cls} _{prod};\n")
                    defined.add(prod)

        f.write("\n")
        f.write("    switch (current_tok) {\n")

        # Group productions by their action
        cases = defaultdict(list)
        for (nt_key, token), production in self.table.items():
            if nt_key == nt:
                token_enum = (
                    f"TOKEN_{token.upper()}"
                    if token != self.end_marker
                    else f"TOKEN_{self.end_marker.upper()}"
                )
                cases[tuple(production)].append(token_enum)

        # Generate case statements
        for production, tokens in cases.items():
            # Create case labels
            for token_enum in sorted(tokens):
                f.write(f"        case {token_enum}:\n")
            f.write("            // Production: " + " ".join(production) + "\n")

            # Handle epsilon production
            if production == (self.epsilon,):
                f.write("            /* epsilon */\n")
                # epsilon might have kode
                for kode in self.code.get((nt, (self.epsilon,)), []):
                    f.write(f"            {kode}\n")
                f.write("            break;\n\n")
                continue

            # Generate production actions
            for symbol in production:
                if symbol in self.terminals and symbol != self.epsilon:
                    f.write(f"            _{symbol} = match_token(TOKEN_{symbol.upper()});\n")
                elif symbol in self.non_terminals:
                    f.write(f"            _{symbol} = {self.nt_to_func[symbol]}();\n")
            for kode in self.code.get((nt, tuple(production)), []):
                # naively dumping unsanitized code into our parser!
                f.write(f"            {kode}\n")
            f.write("            break;\n\n")

        # Default error case
        f.write("        default:\n")
        f.write('            syntax_error("Unexpected token");\n')
        f.write("            recover_from_error(sync_set, sync_size);\n")
        f.write("            break;\n")
        f.write("    }\n")
        f.write("    if ((node != NULL) && (current_index > 0) && (current_tok != TOKEN_EOF)) {\n")
        f.write("        Token token = _current_token();\n")
        f.write("        node->line = token.line;\n")
        f.write("        node->column = token.column;\n")
        f.write("    }\n")
        f.write("    return node;\n")
        f.write("}\n\n")

    def _generate_main_parser(self, f):
        """Generate main parsing function."""
        start_func = self.nt_to_func[self.parser.start_symbol]

        f.write(
            parser_main.format(
                start_func=start_func, ast_name=self.ast_name
            )
        )


# Example usage
if __name__ == "__main__":
    # Example grammar
    with open("dsl.helk") as file:
        dsl_text = file.read()
    parser_gen = DSLProcessor(dsl_text).generate_generator()

    # Generate C code
    code_gen = LL1CCodeGenerator(parser_gen)
    code_gen.generate_parser_code("../out")
    print("C parser code generated successfully")
