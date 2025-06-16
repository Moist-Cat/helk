from textwrap import dedent
import re
from collections import deque, defaultdict

from parsing.parser import LL1ParserGenerator


class DSLProcessor:
    """Processes a domain-specific language (DSL) defining a context-free grammar."""

    def __init__(self, dsl_text, epsilon="ñ", end_marker="EOF"):
        self.dsl_text = dsl_text
        self.epsilon = epsilon
        self.end_marker = end_marker
        self.grammar = {}
        self.non_terminals = []
        self.terminals = set()
        self.code = {}  # non_terminal, kode
        self.associativity = {}
        self.precedence = defaultdict(list)
        self.current_precedence = 0

    def parse_dsl(self):
        """Parse the DSL text into grammar rules."""
        # The @ is not necessary, but its easier to use a separator
        lines = self.dsl_text.strip().split("@")
        current_nt = None
        precedence_levels = {}
        associativity_rules = {}

        for line in lines:
            line = line.strip()
            if not line or line.startswith("#"):
                continue

            # Grammar rule processing
            if ":" in line:
                # left-hand side
                lhs, rhs = line.split(":", 1)
                lhs = lhs.strip()
                rhs = rhs.strip()

                # Add new non-terminal
                if lhs not in self.grammar:
                    self.grammar[lhs] = []
                    self.non_terminals.append(lhs)

                # Split alternatives
                alternatives = [alt.strip() for alt in rhs.split("|")]
                for attrib in alternatives:
                    if "$" in attrib:
                        alt, kode = attrib.split("$", 1)
                    else:
                        alt, kode = attrib, ""
                    alt = alt.strip()

                    tokens = alt.split()
                    if alt == "ε" or alt == "epsilon":
                        self.grammar[lhs].append([self.epsilon])
                        tokens = (self.epsilon,)
                    else:
                        self.grammar[lhs].append(tokens)
                    # strip everyone and delet whites
                    kode = dedent(kode).split("\n")
                    self.code[(lhs, tuple(tokens))] = kode

        # Collect terminals (all symbols not in non_terminals and not epsilon)
        for nt, productions in self.grammar.items():
            for prod in productions:
                for symbol in prod:
                    if (
                        symbol != self.epsilon
                        and symbol not in self.non_terminals
                    ):
                        self.terminals.add(symbol)

        return self.grammar

    def eliminate_left_recursion(self):
        """Eliminate left recursion using standard algorithm."""
        new_grammar = {}
        ordered_nts = self.non_terminals.copy()

        for i, A in enumerate(ordered_nts):
            # Substitute productions
            new_productions = []
            for prod in self.grammar[A]:
                if prod and prod[0] in ordered_nts[:i]:
                    B = prod[0]
                    for b_prod in new_grammar.get(B, self.grammar[B]):
                        if b_prod == [self.epsilon]:
                            new_productions.append(prod[1:])
                        else:
                            new_productions.append(b_prod + prod[1:])
                else:
                    new_productions.append(prod)
            new_grammar[A] = new_productions

            # Eliminate immediate left recursion
            alpha = []
            beta = []
            for prod in new_grammar[A]:
                if prod and prod[0] == A:
                    alpha.append(prod[1:])
                else:
                    beta.append(prod)

            if alpha:
                A_prime = A + "Tail"
                self.non_terminals.append(A_prime)

                # Update productions
                new_grammar[A] = [prod + [A_prime] for prod in beta]
                new_grammar[A_prime] = [prod + [A_prime] for prod in alpha] + [
                    [self.epsilon]
                ]

        # Update grammar with processed rules
        for nt in ordered_nts:
            if nt in new_grammar:
                self.grammar[nt] = new_grammar[nt]

        # Add any new non-terminal productions
        for nt, prods in new_grammar.items():
            if nt not in self.grammar:
                self.grammar[nt] = prods

        return self.grammar

    def left_factor(self):
        """Perform left factoring on grammar."""
        new_grammar = {}
        changed = True

        while changed:
            changed = False
            for A in list(self.grammar.keys()):
                productions = self.grammar[A]
                prefix_map = defaultdict(list)

                # Group productions by common prefix
                for prod in productions:
                    if not prod or prod == [self.epsilon]:
                        prefix_map[tuple()].append(prod)
                    else:
                        prefix_map[(prod[0],)].append(prod)

                # Check for common prefixes
                new_productions = []
                for prefix, prods in prefix_map.items():
                    if len(prods) > 1 and prefix:  # Factorable group
                        changed = True
                        common_prefix = list(prefix)
                        suffixes = [
                            prod[len(common_prefix) :] or [self.epsilon]
                            for prod in prods
                        ]

                        # Create new non-terminal
                        A_prime = A + "Tail"
                        counter = 1
                        while A_prime in self.grammar or A_prime in self.non_terminals:
                            A_prime = A + "Tail" * (counter + 1)
                            counter += 1

                        # Add new non-terminal
                        self.non_terminals.append(A_prime)
                        new_grammar[A_prime] = suffixes

                        # Add factored production
                        new_productions.append(common_prefix + [A_prime])
                    else:
                        new_productions.extend(prods)

                self.grammar[A] = new_productions

            # Update grammar with new productions
            for nt, prods in new_grammar.items():
                if nt not in self.grammar:
                    self.grammar[nt] = prods
            new_grammar = {}

        return self.grammar

    def generate_generator(self):
        """Generate parser initialization code."""
        self.parse_dsl()
        # We don't change the grammar to remove left recursion &c
        # because it will generate new non-terminals and the user
        # can not write code for these
        # The table will explode the moment we give it something not
        # LL1 so we can run the algorithm and manually change the grammar
        # in that case

        start_symbol = self.non_terminals[0]
        epsilon = self.epsilon
        end_marker = self.end_marker
        grammar = self.grammar

        parser = LL1ParserGenerator(
            grammar, start_symbol, epsilon, end_marker, code=self.code
        )

        parser.print_parsing_table()

        return parser

def verify_left_recursion_elimination():
    print("=== Grammar Transformation Tests ===")

    test_grammar = {
        # Left recursion cases
        "A": [["A", "a"], ["b"]],                    # Immediate left recursion
        "B": [["C", "b"], ["A", "c"]],               # Indirect left recursion
        "C": [["B", "d"], ["e"]],
        
        # Left factoring cases
        "D": [["x", "y", "z"], ["x", "w"]],         # Common prefix 'x'
        "E": [["F", "+", "E"], ["F", "-", "E"], ["F"]], # Common prefix 'F'
        "F": [["(", "E", ")"], ["id"]],              # No factoring needed
        
        # Mixed case
        "G": [["G", "a", "b"], ["G", "a", "c"], ["d"]] # Left recursion + common prefix
    }
    
    # Original grammar
    print("\nOriginal Grammar:")
    for nt, prods in test_grammar.items():
        print(f"{nt} → {' | '.join([' '.join(p) for p in prods])}")
    
    # Initialize processor
    processor = DSLProcessor("", epsilon="ε")
    processor.grammar = test_grammar
    processor.non_terminals = list(test_grammar.keys())
    processor.terminals = {'a','b','c','d','e','w','x','y','z','+','-','(',')','id','ε'}
    
    # Step 1: Left recursion elimination
    print("\nAfter Left Recursion Elimination:")
    no_recursion_grammar = processor.eliminate_left_recursion()
    for nt, prods in no_recursion_grammar.items():
        print(f"{nt} → {' | '.join([' '.join(p) for p in prods])}")
    
    # Step 2: Left factoring
    print("\nAfter Left Factoring:")
    final_grammar = processor.left_factor()
    for nt, prods in final_grammar.items():
        print(f"{nt} → {' | '.join([' '.join(p) for p in prods])}")
    
    # Validation checks
    print("\nValidation Results:")
    
    # Check 1: No left recursion remains
    recursion_found = False
    for nt in final_grammar:
        for prod in final_grammar[nt]:
            if prod and prod[0] == nt:
                print(f"❌ Left recursion remains in {nt} → {' '.join(prod)}")
                recursion_found = True
    if not recursion_found:
        print("✅ No left recursion remains")
    
    # Check 2: No common prefixes (except in factored rules)
    factoring_needed = False
    for nt in final_grammar:
        productions = final_grammar[nt]
        first_symbols = [prod[0] for prod in productions if prod]
        if len(set(first_symbols)) < len(first_symbols) and not nt.endswith("'"):
            print(f"❌ Unfactored common prefix in {nt}")
            factoring_needed = True
    if not factoring_needed:
        print("✅ All common prefixes properly factored")
    
    # Check 3: Original language preserved
    # (Would require actual parsing tests with sample inputs)
    print("⚠️  Language preservation requires parsing tests")

# Example usage
if __name__ == "__main__":
    dsl_text = """
    Expr: Term ExprTail @
    ExprTail: PLUS Term ExprTail | MINUS Term ExprTail | epsilon @
    Term: Factor TermTail @
    TermTail: MUL Factor TermTail | DIV Factor TermTail | epsilon @
    Factor: NUM | ID | LPAREN Expr RPAREN @
    """

    processor = DSLProcessor(dsl_text)
    grammar = processor.parse_dsl()
    print("Original Grammar:")
    for nt, prods in grammar.items():
        print(f"{nt} → {' | '.join([' '.join(p) for p in prods])}")

    print("\nAfter Left Recursion Elimination:")
    grammar = processor.eliminate_left_recursion()
    for nt, prods in grammar.items():
        print(f"{nt} → {' | '.join([' '.join(p) for p in prods])}")

    print("\nAfter Left Factoring:")
    grammar = processor.left_factor()
    for nt, prods in grammar.items():
        print(f"{nt} → {' | '.join([' '.join(p) for p in prods])}")

    # Create parser and validate
    processor = DSLProcessor(dsl_text)
    parser = LL1ParserGenerator(grammar)

    parser = processor.generate_generator()
    # Run the test
    verify_left_recursion_elimination()
