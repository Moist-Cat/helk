from pathlib import Path
from collections import deque, defaultdict
from dataclasses import dataclass, field
from typing import Dict, Set, FrozenSet, List, Tuple, Optional

from lexing.condition import (
    Condition,
    SingleCharCondition,
    CharSetCondition,
    WildcardCondition,
    MetaCharCondition,
)


@dataclass
class NFAState:
    """Represents a state in the NFA."""

    # 0 = start; 1 = end;
    # end => accepting is not always True
    id: int
    is_accepting: bool = False
    token_type: Optional[str] = None
    # traks the priority of the pattern; not used but it's okay to keep it around
    pattern_index: Optional[int] = None
    char_transitions: Dict[Condition, Set["NFAState"]] = field(default_factory=dict)
    epsilon_transitions: Set["NFAState"] = field(default_factory=set)

    # notice we can not compare states directly
    def __hash__(self):
        return id(self)

    def __eq__(self, other):
        return id(self.id) == id(other.id)


class NFA:
    """Non-deterministic Finite Automaton representation."""

    def __init__(self, start: NFAState, end: NFAState):
        self.start = start
        self.end = end
        self.states = set()

    def add_state(self, state: NFAState):
        self.states.add(state)

    @staticmethod
    def from_char(condition: Condition) -> "NFA":
        """Create NFA for a single character condition."""
        start = NFAState(0)
        end = NFAState(1, is_accepting=True)
        # OR
        # no need to add a bunch of epsilon transitions
        # for this
        for symbol in condition.expand():
            start.char_transitions[symbol] = {end}
        nfa = NFA(start, end)
        nfa.add_state(start)
        nfa.add_state(end)
        return nfa

    @staticmethod
    def concatenate(nfa1: "NFA", nfa2: "NFA") -> "NFA":
        """Concatenate two NFAs: nfa1 then nfa2."""
        # The famous "pegar dos automatas con 'teipe'" algorithm
        nfa1.end.epsilon_transitions.add(nfa2.start)
        nfa1.end.is_accepting = False
        nfa = NFA(nfa1.start, nfa2.end)
        # Nothing weird here; we just put all states
        # in the final automaton (nfa)
        nfa.states = nfa1.states | nfa2.states
        return nfa

    @staticmethod
    def union(nfas: List["NFA"]) -> "NFA":
        """Create union of multiple NFAs (alternatives)."""
        start = NFAState(0)
        end = NFAState(1, is_accepting=True)
        nfa = NFA(start, end)
        nfa.add_state(start)
        nfa.add_state(end)

        # Basically, we simply add transitions
        # to
        for sub_nfa in nfas:
            start.epsilon_transitions.add(sub_nfa.start)
            sub_nfa.end.epsilon_transitions.add(end)
            sub_nfa.end.is_accepting = False
            nfa.states |= sub_nfa.states
        return nfa

    @staticmethod
    def star(nfa_in: "NFA") -> "NFA":
        """Apply Kleene star (0 or more repetitions)."""
        end = NFAState(1, is_accepting=True)
        nfa_in.end.is_accepting = False

        end.epsilon_transitions.add(nfa_in.start)
        nfa_in.end.epsilon_transitions.add(end)

        # End and start are the one and the same
        # Hopefully this doesn't break anything
        nfa = NFA(end, end)
        nfa.states = {end} | nfa_in.states
        return nfa

    @staticmethod
    def plus(nfa_in: "NFA") -> "NFA":
        """Apply plus (1 or more repetitions)."""
        return NFA.concatenate(nfa_in, NFA.star(nfa_in))

    @staticmethod
    def optional(nfa_in: "NFA") -> "NFA":
        """Apply optional operator (0 or 1 repetitions)."""
        start = NFAState(0)
        end = NFAState(1, is_accepting=True)
        nfa_in.end.is_accepting = False

        start.epsilon_transitions.add(nfa_in.start)
        start.epsilon_transitions.add(end)
        nfa_in.end.epsilon_transitions.add(end)

        nfa = NFA(start, end)
        nfa.states = {start, end} | nfa_in.states
        return nfa

    def __str__(self):
        return f"<NFA: ({self.start=}, {self.end=})>"

    __repr__ = __str__


class DFAConverter:
    """Converts NFA to DFA using subset construction algorithm."""

    def __init__(self, nfa: NFA, alphabet: Set[str]):
        """
        Args:
            nfa_start: Starting state of the NFA
            alphabet: Input symbols (excluding epsilon)
            all_nfa_states: All states in the NFA
        """
        self.nfa_start = nfa.start
        self.alphabet = alphabet
        self.all_nfa_states = nfa.states

        # DFA components
        self.dfa_states: Set[FrozenSet[NFAState]] = set()
        self.dfa_start: Optional[FrozenSet[NFAState]] = None
        self.dfa_accept: Set[FrozenSet[NFAState]] = set()
        # since we mashed together a bunch of (NF) automata, we need another
        # table to keep track of the transitions
        # using NFA.char_transitions adds ambiguity
        self.transitions: Dict[
            Tuple[FrozenSet[NFAState], str], FrozenSet[NFAState]
        ] = {}

        # Track token types for states with multiple accept states
        self.token_types: Dict[FrozenSet[NFAState], str] = {}

    def _epsilon_closure(self, states: Set[NFAState]) -> FrozenSet[NFAState]:
        """Compute epsilon-closure for a set of NFA states."""
        stack = list(states)
        closure = set(states)

        # XXX duplicate code
        # this is in the regex engine as well

        while stack:
            state = stack.pop()
            for neighbor in state.epsilon_transitions:
                if neighbor not in closure:
                    closure.add(neighbor)
                    stack.append(neighbor)
        return frozenset(closure)

    def _move(self, states: Set[NFAState], symbol: Condition) -> Set[NFAState]:
        """Compute move for a set of states under a symbol."""
        next_states = set()
        for state in states:
            if symbol in state.char_transitions:
                next_states |= state.char_transitions[symbol]
        return next_states

    def convert(self) -> None:
        """Perform NFA to DFA conversion using subset construction."""
        # Initialize with epsilon-closure of start state
        start_set = self._epsilon_closure({self.nfa_start})
        self.dfa_start = start_set
        self.dfa_states.add(start_set)

        # Queue for unprocessed DFA states
        queue = deque([start_set])

        while queue:
            current_dfa_state = queue.popleft()

            # Check if this DFA state contains any accepting NFA states
            accept_states = [s for s in current_dfa_state if s.is_accepting]
            if accept_states:
                self.dfa_accept.add(current_dfa_state)
                # For lexer: track token type using first accept state found
                # Since we add the rules to the final NFA in order, the order
                # is kept. This allows us to add priority to rules.
                # If it wasn't the case, we will have an index (pattern_index)
                # to sort the array
                if current_dfa_state not in self.token_types:
                    accept_sorted = sorted(
                        accept_states,
                        key=lambda s: s.pattern_index,
                    )
                    self.token_types[current_dfa_state] = accept_sorted[0].token_type

            # Process each symbol in alphabet
            for symbol in self.alphabet:
                # Compute move + epsilon-closure
                moved = self._move(current_dfa_state, symbol)
                next_state = self._epsilon_closure(moved) if moved else frozenset()

                if not next_state:
                    continue  # No transition for this symbol

                # Add to transitions table
                self.transitions[(current_dfa_state, symbol)] = next_state

                # Add new state to processing queue
                if next_state not in self.dfa_states:
                    self.dfa_states.add(next_state)
                    queue.append(next_state)

        self._prune_unreachable()

    def _prune_unreachable(self) -> None:
        """Remove unreachable DFA states from the state set."""
        # Find reachable states through BFS
        reachable = set()
        queue = deque([self.dfa_start])

        while queue:
            state = queue.popleft()
            if state in reachable:
                continue
            reachable.add(state)

            for symbol in self.alphabet:
                next_state = self.transitions.get((state, symbol))
                if next_state and (next_state not in reachable):
                    queue.append(next_state)

        # Update state sets to only include reachable states
        self.dfa_states = reachable
        self.dfa_accept = {s for s in self.dfa_accept if s in reachable}
        self.transitions = {
            k: v
            for k, v in self.transitions.items()
            if k[0] in reachable and v in reachable
        }

    def get_dfa_components(
        self,
    ) -> Tuple[
        Set[FrozenSet[NFAState]],
        FrozenSet[NFAState],
        Set[FrozenSet[NFAState]],
        Dict[Tuple[FrozenSet[NFAState], Condition], FrozenSet[NFAState]],
    ]:
        """Return DFA components: states, start, accept, transitions."""
        # quite useless except for printing the table
        return (self.dfa_states, self.dfa_start, self.dfa_accept, self.transitions)

    def display_transition_table(self) -> None:
        """Print readable DFA transition table."""
        print("DFA Transition Table:")
        # the '<' thingy adds spaces
        print(f"{'State':<15} | {'Token':<6} | {'Accept':<6} | {'Transitions'}")
        print("-" * 60)

        # Sort states for consistent display
        sorted_states = sorted(
            self.dfa_states, key=lambda s: min(state.id for state in s)
        )

        for state in sorted_states:
            state_label = "{" + ",".join(str(s.id) for s in state) + "}"
            token_type = self.token_types.get(state, "")
            is_accept = "Yes" if state in self.dfa_accept else "No"

            trans = []
            for symbol in self.alphabet:
                if (state, symbol) in self.transitions:
                    target = self.transitions[(state, symbol)]
                    target_label = "{" + ",".join(str(s.id) for s in target) + "}"
                    trans.append(f"{symbol}→{target_label}")

            print(
                f"{state_label:<15} | {token_type:<6} | {is_accept:<6} | {', '.join(trans)}"
            )


# the amalgamation of all (lexical) things
class DFACCodeGenerator:
    """Generates optimized C code to simulate DFA behavior with token return."""

    def __init__(self, converter, token_types: List[str]):
        """
        Args:
            converter: DFAConverter instance with computed DFA
            token_types: Ordered list of token type names
        """
        self.converter = converter
        self.dfa_states = converter.dfa_states
        self.dfa_start = converter.dfa_start
        self.transitions = converter.transitions
        self.token_types = token_types

        # Map state sets to integer IDs
        self.state_to_id = {}
        # no need to sort again
        # self.states_sorted = self.dfa_states
        self.states_sorted = self.dfa_states
        #self.states_sorted = sorted(
        #    self.dfa_states,
        #    key=lambda s: min(
        #        state.pattern_index if state.pattern_index is not None else float("inf")
        #        for state in s
        #    ),
        #)

        for i, state_set in enumerate(self.states_sorted):
            self.state_to_id[state_set] = i

        # Precompute accepting states and token IDs
        self.accepting = []
        self.token_ids = []
        for state_set in self.states_sorted:
            if state_set in converter.token_types:
                token_name = converter.token_types[state_set]
                self.accepting.append(1)
                self.token_ids.append(token_types.index(token_name))
            else:
                self.accepting.append(0)
                self.token_ids.append(-1)

    def _c_escape_char(self, c: int) -> str:
        """Convert char code to escaped C character literal."""
        if c == ord("\\"):
            return r"'\\'"
        if c == ord("'"):
            return r"'\''"
        if c == ord('"'):
            return r"'\"'"  # Safe in single quotes
        if c == ord("\0"):
            return r"'\0'"
        if c == ord("\n"):
            return r"'\n'"
        if c == ord("\t"):
            return r"'\t'"
        if c == ord("\r"):
            return r"'\r'"
        if 32 <= c <= 126:  # Printable ASCII
            return f"'{chr(c)}'"
        return f"'\\x{c:02x}'"  # Hex escape for non-printables

    def _get_ranges(self, chars: List[int]) -> List[Tuple[int, int]]:
        """Convert sorted char codes to inclusive ranges."""
        if not chars:
            return []
        sorted_chars = sorted(chars)
        ranges = []
        start = end = sorted_chars[0]

        for c in sorted_chars[1:]:
            if c == end + 1:
                end = c
            else:
                ranges.append((start, end))
                start = end = c
        ranges.append((start, end))
        return ranges

    def generate_header(self, h_file: Path) -> None:
        """Generate .h file with function declarations and token types."""
        with open(h_file / "regex_dfa.h", "w") as f:
            f.write("#ifndef REGEX_DFA_H\n")
            f.write("#define REGEX_DFA_H\n\n")

            # Token type definitions
            f.write("// Token type definitions\n")
            f.write("typedef enum {\n")
            for i, token in enumerate(self.token_types):
                f.write(f"    TOKEN_{token.upper()},\n")
            f.write("    TOKEN_EOF,\n")
            f.write("    TOKEN_ERROR,\n")
            f.write("} TokenType;\n\n")

            # Function declaration
            f.write("// DFA matching function\n")
            f.write(
                "const char* match_pattern(const char* input, TokenType* token_type);\n\n"
            )
            f.write("#endif // REGEX_DFA_H\n")

    def generate_source(self, c_file: Path) -> None:
        """Generate .c file with DFA simulation implementation."""
        with open(c_file / "regex_dfa.c", "w") as f:
            # Include header and start function
            f.write(f'#include "regex_dfa.h"\n')
            f.write(f"#include <stdio.h>\n\n")
            f.write(
                "const char* match_pattern(const char* input, TokenType* token_type) {\n"
            )
            f.write("    const char* current = input;\n")
            f.write("    const char* last_accept = NULL;\n")
            f.write("    TokenType last_token = TOKEN_ERROR;\n")
            f.write("    char c;\n\n")

            # Start at initial state
            start_id = self.state_to_id[self.dfa_start]
            f.write(f"    goto STATE_{start_id};\n\n")

            # Generate state handlers
            for i, state_set in enumerate(self.states_sorted):
                f.write(f"STATE_{i}:\n")

                # Update last accept position
                if self.accepting[i]:
                    nam = self.token_types[self.token_ids[i]].upper()
                    #for state in state_set:
                    #    if state.is_accepting and (state.token_type != (self.token_types[self.token_ids[i]].upper())):
                    #        nam = state.token_type
                    f.write(f"    last_accept = current;\n")
                    f.write(
                        f"    last_token = TOKEN_{self.token_types[self.token_ids[i]].upper()};\n"
                        #f"    last_token = TOKEN_{nam};\n"
                    )

                # we could add another state for this
                # but it would prefer not to add more jumps
                # End of string handling
                f.write("    if (*current == (char) '\\0') {\n")
                f.write("        if (last_accept != NULL) {\n")
                f.write("            *token_type = last_token;\n")
                f.write("            return current;\n")
                f.write("        }\n")
                f.write("        *token_type = TOKEN_ERROR;\n")
                f.write("        return current;\n")
                f.write("    }\n")

                f.write("    c = *current++;\n")

                # Build transition map for current state
                next_map = defaultdict(list)
                for c_int in range(128):  # (traditional) ASCII
                    char = chr(c_int)
                    key = (state_set, char)
                    if key in self.transitions:
                        next_state = self.transitions[key]
                        next_id = self.state_to_id[next_state]
                        next_map[next_id].append(c_int)

                # Generate optimized condition checks
                conditions_generated = False
                for next_id, chars in next_map.items():
                    ranges = self._get_ranges(chars)
                    condition_parts = []

                    for start, end in ranges:
                        if start == end:
                            c_repr = self._c_escape_char(start)
                            # char literals in C are ints
                            # figures
                            condition_parts.append(f"c == (char) {c_repr}")
                        else:
                            start_repr = self._c_escape_char(start)
                            end_repr = self._c_escape_char(end)
                            condition_parts.append(
                                f"(c >= {start_repr} && c <= {end_repr})"
                            )

                    condition_str = " || ".join(condition_parts)
                    f.write(f"    if ({condition_str}) ")
                    # no newline; next to the if
                    f.write(f"goto STATE_{next_id};\n")
                    conditions_generated = True

                # Handle dead state transitions
                if not conditions_generated:
                    f.write("    goto DEAD;\n")
                else:
                    f.write("    else goto DEAD;\n")
                f.write("\n")

            # Dead state handler
            f.write("DEAD:\n")
            f.write("    if (last_accept != NULL) {\n")
            f.write("        *token_type = last_token;\n")
            f.write("        return last_accept;\n")
            f.write("    }\n")
            f.write("    return NULL;\n")
            f.write("}\n")

    def generate_code(self, output_dir: str) -> None:
        """Generate complete C implementation."""
        self.generate_header(Path(output_dir))
        self.generate_source(Path(output_dir))


def create_nfa_for_pattern() -> Tuple[NFAState, Set[NFAState]]:
    """Create NFA for regex pattern: a(b|c)*"""
    # Create states
    q0 = NFAState(0)
    q1 = NFAState(1)
    q2 = NFAState(2, is_accepting=True, token_type="FINAL")

    # Add transitions
    q0.char_transitions["a"] = {q1}
    q1.epsilon_transitions = {q2}
    q2.char_transitions["b"] = {q2}
    q2.char_transitions["c"] = {q2}

    nfa = NFA(start=q0, end=q2)

    return nfa


# Test the implementation
if __name__ == "__main__":
    # Create NFA components
    nfa = create_nfa_for_pattern()
    alphabet = {"a", "b", "c"}

    # Convert to DFA
    converter = DFAConverter(nfa, alphabet)
    converter.convert()

    # Display results
    print("NFA to DFA Conversion for Pattern: a(b|c)*")
    converter.display_transition_table()

    # Get DFA components
    dfa_states, dfa_start, dfa_accept, transitions = converter.get_dfa_components()
    print(f"\nDFA States: {len(dfa_states)}")
    print(f"Start State: {{{','.join(str(s.id) for s in dfa_start)}}}")
    print(f"Accept States: {len(dfa_accept)}")

    # Generate C code
    code_gen = DFACCodeGenerator(
        converter,
        [
            "FINAL",
        ],
    )
    code_gen.generate_code(".")
    print("C code generated successfully")
