import sys
from pathlib import Path
from dataclasses import dataclass
from typing import Dict, List, Optional, Set, Tuple, Callable, Union
import os
import re
from collections import defaultdict
from textwrap import dedent

from lexing.condition import SingleCharCondition, MetaCharCondition
from lexing.regex import RegexEngine
from lexing.automata import NFA, NFAState, DFAConverter, DFACCodeGenerator

BASE_DIR = Path(__file__).parent

with open(BASE_DIR / "lexer_template.txt") as file:
    lexer_template = file.read()

with open(BASE_DIR / "lexer_h_template.txt") as file:
    lexer_h_template = file.read()

@dataclass
class Token:
    type: str
    value: str
    line: int
    column: int

class Lexer:
    """Lexer generator with regex-based tokenization and position tracking."""
    def __init__(self, rules: List[Tuple[str, str]], skip_whitespace: bool = True):
        self.rules = rules
        self.skip_whitespace = skip_whitespace
        self.engine = RegexEngine()
        self.combined_nfa = self._build_combined_nfa()
        self.line = 1
        self.column = 1
        self.cache: Dict[int, Tuple] = {}  # Cache token matches by position

    def _build_combined_nfa(self) -> NFA:
        """Combine all rule NFAs into a single NFA with priorities."""
        start = NFAState(0)
        nfa = NFA(start, start)
        nfa.add_state(start)
        
        # Build NFAs for each pattern and connect to start
        for idx, (token_type, pattern) in enumerate(self.rules):
            pattern_nfa = self.engine.compile(pattern)
            pattern_nfa.end.is_accepting = True
            pattern_nfa.end.token_type = token_type
            pattern_nfa.end.pattern_index = idx
            start.epsilon_transitions.add(pattern_nfa.start)
            nfa.states |= pattern_nfa.states
        
        return nfa

    def tokenize(self, text: str) -> List[Token]:
        """Convert input text into tokens with position tracking."""
        tokens = []
        index = 0
        length = len(text)
        
        while index < length:
            if self.skip_whitespace:
                # Skip whitespace
                ws_count = 0
                while index + ws_count < length and text[index + ws_count] in ' \t':
                    ws_count += 1
                if ws_count:
                    self._update_pos(text[index:index+ws_count])
                    index += ws_count
                    continue
            
            # Get next token
            token = self._next_token(text, index)
            if not token:
                # Handle unmatched text
                raise SyntaxError(f"Unexpected character '{text[index]}' at line {self.line}, col {self.column}")
            
            # Create token and update position
            token_type, token_value, token_length = token
            tokens.append(Token(token_type, token_value, self.line, self.column))
            self._update_pos(token_value)
            index += token_length
        
        return tokens

    def _next_token(self, text: str, start_index: int) -> Optional[Tuple[str, str, int]]:
        """Find the next token match using the combined NFA."""
        if start_index in self.cache:
            return self.cache[start_index]
        
        current_states = self.engine._epsilon_closure({self.combined_nfa.start})
        best_match = None
        index = start_index
        
        # Traverse while we have valid states
        while index < len(text) and current_states:
            char = text[index]
            current_states = self.engine._epsilon_closure(
                self.engine._step(current_states, char))
            index += 1
            
            # Check for accepting states (potential tokens)
            for state in current_states:
                if state.is_accepting:
                    # Prioritize: longest match then earliest pattern
                    token_length = index - start_index
                    if best_match is None or token_length > best_match[2] or \
                       (token_length == best_match[2] and 
                        state.pattern_index < best_match[3]):
                        best_match = (
                            state.token_type,
                            text[start_index:index],
                            token_length,
                            state.pattern_index
                        )
        
        # Cache and return best match
        result = (best_match[:3]) if best_match else None
        self.cache[start_index] = result
        return result

    def _update_pos(self, token_value: str):
        """Update line/column position based on token content."""
        if '\n' not in token_value:
            self.column += len(token_value)
        else:
            lines = token_value.split('\n')
            self.line += len(lines) - 1
            self.column = len(lines[-1]) + 1  # +1 for next position

    def generate_c_lexer(self, output_dir: str):
        """
        Generate C lexer (lexer.h and lexer.c) from Python lexer rules
        Handles regex patterns, position tracking, and prioritized rules
        """
        output_dir = Path(output_dir)
        os.makedirs(output_dir, exist_ok=True)

        ascii_set = {chr(i) for i in range(128)}
        converter = DFAConverter(self.combined_nfa, ascii_set)
        converter.convert()
        converter.display_transition_table()
        

        # ordered
        tokens = list(dict(self.rules).keys())

        generator = DFACCodeGenerator(
            converter,
            tokens,
        )
        generator.generate_code(output_dir)
        
        # Generate lexer.h
        with open(os.path.join(output_dir, "lexer.h"), "w") as f:
            f.write(self._generate_header())
        
        # Generate lexer.c
        with open(os.path.join(output_dir, "lexer.c"), "w") as f:
            f.write(self._generate_source())

    def _generate_header(self) -> str:
        """Generate C header file with token definitions and function declarations"""
        token_types = ["TOKEN_{}".format(rule[0].upper()) for rule in self.rules]
        token_types.extend(["TOKEN_EOF", "TOKEN_ERROR"])
        
        return dedent(lexer_h_template).format(token_enums=",\n    ".join(token_types))

    def _generate_source(self) -> str:
        """Generate C source file with lexer implementation and regex handlers"""
        rule_functions = []
        rule_types = []
        
        return dedent(lexer_template).format(
            rule_functions="\n\n".join(rule_functions),
            rule_func_ptrs=", ".join([f"rule{i}" for i in range(len(self.rules))]),
            rule_type_list=", ".join(rule_types),
            num_rules=len(self.rules)
        )

# ======================
# Unit Tests
# ======================
def run_tests():
    """Run comprehensive unit tests."""
    # Regex Engine Tests
    engine = RegexEngine()
    
    # Literals and sequences
    assert engine.match("a", "a")
    assert not engine.match("a", "b")
    assert engine.match("abc", "abc")
    assert not engine.match("abc", "ab")
    
    # Character classes
    assert engine.match("[a-z]", "g")
    assert not engine.match("[a-z]", "5")
    assert engine.match("[^0-9]", "x")
    assert not engine.match("[^0-9]", "5")
    
    # Wildcard
    assert engine.match(".", "x")
    assert not engine.match(".", "\n")
    
    # Escaped characters
    assert engine.match("\\n", "\n")
    assert engine.match("\\d", "5")
    assert not engine.match("\\d", "x")
    assert engine.match("\\w", "_")
    assert engine.match("\\s", " ")
    
    # Alternation and grouping
    assert engine.match("a|b", "a")
    assert engine.match("a|b", "b")
    assert not engine.match("a|b", "c")
    assert engine.match("(ab)+", "abab")
    assert not engine.match("(ab)+", "aba")

    # Operators
    assert engine.match("a*", "")
    assert engine.match("a*", "aaa")
    assert engine.match("a+", "a")
    assert not engine.match("a+", "")
    assert engine.match("a?b", "b")
    assert engine.match("a?b", "ab")
    assert not engine.match("a?b", "aab")
    
    # Complex patterns
    assert engine.match("\\d+\\.\\d+", "123.45")
    assert not engine.match("\\d+\\.\\d+", "123.")
    assert engine.match("#[^\n]*", "# Comment")
    
    # Lexer Tests
    lexer = Lexer([
        ("FLOAT", r"\d+\.\d+"),
        ("INT", r"\d+"),
        ("ID", r"[a-zA-Z_][a-zA-Z0-9_]*"),
        ("ASSIGN", r"="),
        ("COMMENT", r"#[^\n]*"),
        ("SKIP", r"[ \t]+")
    ], skip_whitespace=True)
    
    # Tokenization
    tokens = lexer.tokenize("price = 123.45 # USD")
    types = [t.type for t in tokens]
    assert types == ["ID", "ASSIGN", "FLOAT", "COMMENT"]
    assert tokens[0].value == "price"
    assert tokens[2].value == "123.45"
    assert tokens[3].value == "# USD"
    
    # Position tracking
    assert tokens[0].line == 1 and tokens[0].column == 1
    assert tokens[1].line == 1 and tokens[1].column == 7
    assert tokens[2].line == 1 and tokens[2].column == 9
    assert tokens[3].line == 1 and tokens[3].column == 16
    
    # Longest match and priority
    lexer = Lexer([
        ("FLOAT", r"\d+\.\d+"),
        ("INT", r"\d+"),
        ("IF", r"if")
    ])
    tokens = lexer.tokenize("if 42 3.14")
    assert [t.type for t in tokens] == ["IF", "INT", "FLOAT"]

    ascii_set = {chr(i) for i in range(128)}

    converter = DFAConverter(lexer.combined_nfa, ascii_set)
    converter.convert()
    converter.display_transition_table()
    print("All tests passed!")

if __name__ == "__main__":
    run_tests()
    print("Engine ready for use")

    lexer = Lexer([
        # types
        ("FLOAT", r"\d+\.\d+"),
        ("INT", r"\d+"),

        # kw
        ("NEW", r"new"),
        ("TYPE", r"type"),
        ("FLOAT", r"\d+\.\d+"),
        ("FLOAT", r"\d+\.\d+"),
        ("FLOAT", r"\d+\.\d+"),
        

        # id
        ("ID", r"[a-zA-Z_][a-zA-Z0-9_]*"),

        # ops
        ("ASSIGN", r"="),

        # misc
        ("COMMENT", r"#[^\n]*")

    ], skip_whitespace=True)

    HULK = [
        # String literals (with escape handling)
        ("STRING_LITERAL", r'\"([^"\\]|\\.)*\"'),

        # Numeric literals (integers only per flex rules)
        ("NUMBER", r'\d+'),

        # Keywords (must come before IDENTIFIER)
        ("NEW", r"new"),
        ("TYPE", r"type"),
        ("INHERITS", r"inherits"),
        ("IF", r"if"),
        ("ELSE", r"else"),
        ("IN", r"in"),
        ("LET", r"let"),
        ("WHILE", r"while"),
        ("FUNCTION", r"function"),

        # Multi-character operators
        ("ARROW", r"=>"),

        # Single-character operators and punctuation
        ("DOT", r"\."),
        ("SEMICOLON", r";"),
        ("COMMA", r","),
        ("EQUALS", r"="),
        ("MOD", r"%"),
        ("EXP", r"\^"),
        ("PLUS", r"\+"),
        ("MINUS", r"-"),
        ("MULTIPLY", r"\*"),
        ("DIVIDE", r"/"),
        ("LPAREN", r"\("),
        ("RPAREN", r"\)"),
        ("LBRACE", r"\{"),
        ("RBRACE", r"\}"),

        # Identifiers
        ("IDENTIFIER", r'[a-zA-Z_][a-zA-Z0-9_]*'),
    ]

    lexer = Lexer(HULK, skip_whitespace=True)
    lexer.generate_c_lexer("../out")
