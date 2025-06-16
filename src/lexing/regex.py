from typing import List, Set, Optional

from lexing.condition import (
    Condition,
    SingleCharCondition,
    CharSetCondition,
    WildcardCondition,
    MetaCharCondition,
)
from lexing.automata import NFA, NFAState


class RegexParser:
    """Converts regex patterns to NFAs using recursive descent parsing (left recursion; 1 token lookahead)."""

    def __init__(self):
        self.state_counter = 0
        self.tokens = []
        self.pos = 0

    def parse(self, pattern: str) -> NFA:
        """Parse a regex pattern into an NFA."""
        self.tokens = self._tokenize(pattern)
        self.pos = 0
        return self._regex()

    def _tokenize(self, pattern: str) -> List[str]:
        """Convert regex pattern into tokens for parsing."""
        tokens = []
        i = 0
        while i < len(pattern):
            c = pattern[i]
            if c == "\\":
                # Handle escape sequences
                i += 1
                if i >= len(pattern):
                    raise ValueError("Trailing backslash")
                esc_char = pattern[i]
                tokens.append(f"\\{esc_char}")
            elif c in "*+?|()":
                tokens.append(c)
            elif c == "[":
                # Handle character classes
                j = i + 1
                negate = False
                if j < len(pattern) and pattern[j] == "^":
                    negate = True
                    j += 1

                class_chars = []
                while j < len(pattern) and pattern[j] != "]":
                    if pattern[j] == "\\":
                        j += 1
                        if j < len(pattern):
                            class_chars.append(f"\\{pattern[j]}")
                    else:
                        class_chars.append(pattern[j])
                    j += 1

                if j >= len(pattern) or pattern[j] != "]":
                    raise ValueError("Unclosed character class")

                tokens.append(f'[{"^" if negate else ""}{"".join(class_chars)}]')
                i = j  # Skip to end of class
            else:
                tokens.append(c)
            i += 1
        return tokens

    def _next_token(self) -> Optional[str]:
        """Get next token without advancing."""
        return self.tokens[self.pos] if self.pos < len(self.tokens) else None

    def _consume(self) -> Optional[str]:
        """Consume and return next token."""
        if self.pos < len(self.tokens):
            token = self.tokens[self.pos]
            self.pos += 1
            return token
        return None

    def _regex(self) -> NFA:
        """Parse a regex pattern (top-level)."""
        terms = []
        while (token := self._next_token()) and token != ")" and token != "|":
            terms.append(self._term())

        nfa = self._build_sequence(terms) if terms else self._parse_atom()

        # Handle alternations
        while self._next_token() == "|":
            self._consume()  # Skip '|'
            terms = [self._term()]
            nfa = NFA.union([nfa, self._build_sequence(terms)])
        return nfa

    def _term(self) -> NFA:
        """Parse a regex term (sequence of factors)."""
        factors = []
        while (token := self._next_token()) and token not in (")", "|", "*", "+", "?"):
            factors.append(self._factor())
        return self._build_sequence(factors)

    def _build_sequence(self, nfas: List[NFA]) -> NFA:
        """Build concatenated NFA from a list of NFAs."""
        if not nfas:
            return self._parse_atom()  # Fallback to atomic element
        result = nfas[0]
        for nfa in nfas[1:]:
            result = NFA.concatenate(result, nfa)
        return result

    def _factor(self) -> NFA:
        """Parse a factor with optional operators (*, +, ?)."""
        atom = self._parse_atom()
        token = self._next_token()
        if token == "*":
            self._consume()
            return NFA.star(atom)
        elif token == "+":
            self._consume()
            return NFA.plus(atom)
        elif token == "?":
            self._consume()
            return NFA.optional(atom)
        return atom

    def _parse_atom(self) -> NFA:
        """Parse an atomic regex element."""
        token = self._consume()
        if not token:
            raise ValueError("Unexpected end of pattern")

        if token == "(":
            nfa = self._regex()
            if self._consume() != ")":
                raise ValueError("Unclosed parenthesis")
            return nfa
        elif token == ".":
            return NFA.from_char(WildcardCondition())
        elif token.startswith("\\"):
            return self._parse_escape(token[1])
        elif token.startswith("["):
            return self._parse_char_class(token[1:-1])
        return NFA.from_char(SingleCharCondition(token))

    def _parse_escape(self, char: str) -> NFA:
        """Parse escape sequences into conditionals."""
        if char in "ntr":
            # Handle special characters
            mapping = {"n": "\n", "t": "\t", "r": "\r"}
            return NFA.from_char(SingleCharCondition(mapping[char]))
        elif char in "dws":
            return NFA.from_char(MetaCharCondition(char))
        # Literal escaped character
        return NFA.from_char(SingleCharCondition(char))

    def _parse_char_class(self, token: str) -> NFA:
        """Parse character class into condition set."""
        negate = False
        if token.startswith("^"):
            negate = True
            token = token[1:]

        chars = set()
        i = 0
        while i < len(token):
            c = token[i]
            if c == "\\":
                i += 1
                if i < len(token):
                    esc_char = token[i]
                    if esc_char in "dws":
                        # Expand metacharacters in class
                        meta_set = MetaCharCondition(esc_char).sets[esc_char]
                        chars |= meta_set
                    else:
                        chars.add(esc_char)
            elif i + 2 < len(token) and token[i + 1] == "-":
                # Handle character ranges
                start = c
                end = token[i + 2]
                chars |= set(chr(code) for code in range(ord(start), ord(end) + 1))
                i += 2  # Skip the range characters
            else:
                chars.add(c)
            i += 1

        return NFA.from_char(CharSetCondition(chars, negate))


class RegexEngine:
    """Compiles and matches regex patterns using NFAs."""

    def __init__(self):
        self.nfa_cache: Dict[str, NFA] = {}  # Cache compiled regexes

    def compile(self, pattern: str) -> NFA:
        """Compile regex pattern to NFA (cached)."""
        if pattern not in self.nfa_cache:
            parser = RegexParser()
            self.nfa_cache[pattern] = parser.parse(pattern)
        return self.nfa_cache[pattern]

    def match(self, pattern: str, text: str) -> bool:
        """Check if text matches the regex pattern from start."""
        nfa = self.compile(pattern)
        # walk the clausure in parallel
        current_states = self._epsilon_closure({nfa.start})

        for char in text:
            current_states = self._epsilon_closure(self._step(current_states, char))
            # The automaton halted
            if not current_states:
                return False
        # Did we halt in a final state?
        return any(state.is_accepting for state in current_states)

    def _step(self, states: Set[NFAState], char: str) -> Set[NFAState]:
        """Move NFA states forward by consuming a character."""
        next_states = set()
        for state in states:
            for condition, targets in state.char_transitions.items():
                if condition == char:
                    next_states |= targets
        return next_states

    def _epsilon_closure(self, states: Set[NFAState]) -> Set[NFAState]:
        """Compute epsilon closure of NFA states."""
        closure = set(states)
        stack = list(states)

        # iterative DFS with a stack
        while stack:
            state = stack.pop()
            for target in state.epsilon_transitions:
                if target not in closure:
                    closure.add(target)
                    stack.append(target)
        return closure
