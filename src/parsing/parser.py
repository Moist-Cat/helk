class LL1ParserGenerator:
    def __init__(
        self, grammar, start_symbol=None, epsilon="ñ", end_marker="EOF", code=None
    ):
        self.grammar = grammar
        self.start_symbol = start_symbol or list(grammar.keys())[0]
        self.epsilon = epsilon
        self.end_marker = end_marker
        self.non_terminals = set(grammar.keys())
        self.terminals = self._compute_terminals()  # non non terminals
        self.code = code or {}

    def _compute_terminals(self):
        terminals = set()
        for nt, prods in self.grammar.items():
            for prod in prods:
                for symbol in prod:
                    if symbol != self.epsilon and symbol not in self.non_terminals:
                        terminals.add(symbol)
        return terminals

    def compute_first(self):
        """
        FIRST set
        """
        first = {}
        for t in self.terminals:
            first[t] = {t}
        first[self.epsilon] = {self.epsilon}
        for nt in self.non_terminals:
            first[nt] = set()

        changed = True
        while changed:
            changed = False
            for nt in self.non_terminals:
                for prod in self.grammar[nt]:
                    all_epsilon = True
                    temp = set()
                    for symbol in prod:
                        # the FIRST of the symbol can change (if nt)
                        # this is the reason its a fixed point algorithm
                        s_first = first.get(symbol, set())
                        non_epsilon = s_first.difference({self.epsilon})
                        # UNION
                        temp = temp | non_epsilon
                        if self.epsilon not in s_first:
                            all_epsilon = False
                            break
                    if all_epsilon:
                        temp.add(self.epsilon)
                    # if it's a subset, it did not change
                    # so this step is necessary
                    if not temp.issubset(first[nt]):
                        first[nt] = first[nt] | temp
                        changed = True
        return first

    def _first_of_sequence(self, seq, first_dict):
        """
        Yields all the FIRSTs of a sequence of symbols
        """
        if not seq:
            return {self.epsilon}
        result = set()
        all_epsilon = True
        for s in seq:
            s_first = first_dict.get(s, set())
            non_epsilon = s_first.difference({self.epsilon})
            result = result | non_epsilon
            if self.epsilon not in s_first:
                all_epsilon = False
                break
        if all_epsilon:
            result.add(self.epsilon)
        return result

    def compute_follow(self, first_dict):
        follow = {nt: set() for nt in self.non_terminals}
        follow[self.start_symbol].add(self.end_marker)

        changed = True
        while changed:
            changed = False
            for nt in self.non_terminals:
                for prod in self.grammar[nt]:
                    for index, symbol in enumerate(prod):
                        if symbol in self.non_terminals:
                            if index < len(prod) - 1:
                                suffix = prod[index + 1 :]
                                first_suffix = self._first_of_sequence(
                                    suffix, first_dict
                                )
                                to_add = first_suffix.difference({self.epsilon})
                                if self.epsilon in first_suffix:
                                    to_add = to_add | follow[nt]
                            else:
                                to_add = follow[nt]

                            if not to_add.issubset(follow[symbol]):
                                follow[symbol] = follow[symbol] | to_add
                                changed = True

        return follow

    def build_parsing_table(self):
        first = self.compute_first()
        follow = self.compute_follow(first)
        table = {}
        for nt in self.non_terminals:
            for prod in self.grammar[nt]:
                first_prod = self._first_of_sequence(prod, first)
                for a in first_prod:
                    if a == self.epsilon:
                        continue
                    key = (nt, a)
                    if key in table:
                        raise Exception(
                            f"Conflict in parsing table at {key}: existing {table[key]}, new {prod} <=> not LL(1)"
                        )
                    table[key] = prod
                if self.epsilon in first_prod:
                    for a in follow[nt]:
                        key = (nt, a)
                        if key in table:
                            raise Exception(
                                f"Conflict in parsing table at {key}: existing {table[key]}, new {prod} <=> not LL(1)"
                            )
                        table[key] = prod
        return table

    def generate_parser(self):
        table = self.build_parsing_table()
        return LL1Parser(
            table,
            self.start_symbol,
            self.non_terminals,
            self.end_marker,
            self.epsilon,
        )

    def print_parsing_table(self, table=None, max_columns: int = 5) -> None:
        """
        Prints the LL(1) parsing table in readable chunks.

        Args:
            table: The parsing table to print (builds one if None)
            max_columns: Maximum number of terminal columns per chunk
        """
        if table is None:
            table = self.build_parsing_table()

        if not table:
            print("Parsing table is empty.")
            return

        # Collect all non-terminals and terminals
        non_terminals = sorted({nt for (nt, _) in table.keys()})
        terminals = sorted({t for (_, t) in table.keys()})

        # Determine column widths
        nt_width = max(len(nt) for nt in non_terminals) if non_terminals else 10
        col_width = max((max(len(t) for t in terminals) + 15 if terminals else 8), 8)

        # Split terminals into chunks
        terminal_chunks = [
            terminals[i : i + max_columns]
            for i in range(0, len(terminals), max_columns)
        ]

        for chunk_num, term_chunk in enumerate(terminal_chunks, 1):
            # Print chunk header
            print(f"\n=== Parsing Table (Part {chunk_num}/{len(terminal_chunks)}) ===")
            print(f"=== Terminals {term_chunk[0]} to {term_chunk[-1]} ===")

            # Print column headers
            header = " " * (nt_width + 2) + "|"
            for t in term_chunk:
                header += f" {t:^{col_width}} |"
            print(header)

            # Print separator line
            separator = "-" * (nt_width + 2) + "+"
            separator += ("-" * (col_width + 2) + "+") * len(term_chunk)
            print(separator)

            # Print each non-terminal row for this chunk
            for nt in non_terminals:
                row = f" {nt:<{nt_width}} |"
                for t in term_chunk:
                    production = table.get((nt, t), None)
                    if production:
                        # Shorten long productions for display
                        prod_str = " ".join(production)
                        if len(prod_str) > col_width - 3:
                            prod_str = prod_str[: col_width - 6] + "..."
                        row += f" {prod_str:^{col_width}} |"
                    else:
                        row += " " * (col_width + 2) + "|"
                print(row)

        print("\nKey:")
        print(
            "- Empty cells indicate parsing errors (no production for that NT and terminal)"
        )
        print("- Productions are shown as right-hand sides of rules")
        if len(terminal_chunks) > 1:
            print(f"- Table split into {len(terminal_chunks)} parts for readability")


if __name__ == "__main__":
    # Define the grammar for arithmetic expressions
    from pprint import pprint

    # pp. 217; dragon
    grammar = {
        "E": [["T", "E'"]],
        "E'": [["+", "T", "E'"], ["ñ"]],
        "T": [["F", "T'"]],
        "T'": [["*", "F", "T'"], ["ñ"]],
        "F": [["(", "E", ")"], ["number"]],
    }

    # Initialize the parser generator
    parser_generator = LL1ParserGenerator(
        grammar, start_symbol="E", epsilon="ñ", end_marker="$"
    )
    pprint(parser_generator.compute_first())
    pprint(parser_generator.compute_follow(parser_generator.compute_first()))

    grammar = {
        "E": [["T", "E'"]],
        "E'": [["+", "T", "E'"], ["ñ"]],
        "T": [["F", "T'"]],
        "T'": [["*", "F", "T'"], ["ñ"]],
        "F": [["(", "E", ")"], ["number"]],
    }

    parser_generator = LL1ParserGenerator(
        grammar, start_symbol="E", epsilon="ñ", end_marker="$"
    )

    parser = parser_generator.generate_parser()

    # Example input tokens: "number + number * number"
    tokens = ["number", "+", "number", "*", "number"]

    # Parse the tokens
    result = parser.parse(tokens)
    print("Parsing successful:", result)  # Expected output: True

    # Another example with parentheses: "( number + number ) * number"
    tokens_with_parens = ["(", "number", "+", "number", ")", "*", "number"]
    result = parser.parse(tokens_with_parens)
    print("Parsing successful:", result)  # Expected output: True

    # Example with invalid input: "number + + number"
    invalid_tokens = ["number", "+", "+", "number"]
    result = parser.parse(invalid_tokens)
    print("Parsing successful:", result)  # Expected output: False
