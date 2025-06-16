from typing import Set

ASCII_SET = {chr(i) for i in range(128)}


class Condition:
    """Base class for character matching conditions."""

    def test(self, char: str) -> bool:
        raise NotImplementedError

    def expand(self) -> list:
        raise NotImplementedError

    def __repr__(self):
        return self.__str__()


class SingleCharCondition(Condition):
    """Matches a specific character."""

    def __init__(self, char: str):
        self.char = char

    def expand(self):
        return [self.char]

    def __str__(self):
        return self.char


class CharSetCondition(Condition):
    """Matches characters from a set (with optional negation)."""

    def __init__(self, chars: Set[str], negate: bool = False):
        self.chars = chars
        self.negate = negate

    def expand(self):
        if not self.negate:
            return self.chars
        return ASCII_SET.difference(self.chars)

    def __str__(self):
        return f"[{'^' if self.negate else ''}{''.join(sorted(self.chars))}]"


class WildcardCondition(Condition):
    """Matches any character (except newline if specified)."""

    def test(self, char: str) -> bool:
        return char != "\n"  # Traditional regex: . doesn't match newline

    def expand(self):
        return ASCII_SET.difference({"\n"})

    def __str__(self):
        return "."


class MetaCharCondition(Condition):
    """Matches character classes via metacharacters (\d, \w, \s)."""

    def __init__(self, meta_type: str):
        self.meta_type = meta_type
        # Define character sets for metacharacters
        self.sets = {
            "d": set("0123456789"),
            "w": set("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_"),
            "s": set(" \t\n\r\f\v"),
        }

    def expand(self):
        return self.sets[self.meta_type]

    def __str__(self):
        return f"\\{self.meta_type}"
