"""Rule-based classification of a SenType-vs-reference difference. First matching rule wins.
Returns (category, rule).

Inputs per case: the engine under comparison (`ref`), method (T/V), the typed `keys`, the three
outputs. `DICT` is the independent hunspell vi_VN syllable list (set by compare.py).
Every rule is documented in docs/testing.md; nothing here changes a result, it only labels it.
"""
import re
import unicodedata

DICT = set()


def words(s):
    return s.split(" ")


def in_dict(word):
    return unicodedata.normalize("NFC", word.lower()) in DICT


def all_dict(s):
    return all(in_dict(w) for w in words(s) if w)


def has_tone(word):
    return any(c in "̣́̀̉̃" for c in unicodedata.normalize("NFD", word))


def classify(ref, method, keys, sentype, refout, other):
    raw_sentype = sentype == keys
    raw_ref = refout == keys

    # 1. Two engines against one: the odd reference is the quirk, not SenType.
    if other == sentype:
        return "reference-quirk", "odd-one-out"

    # 2. VNI digits: SenType ends the word at a digit that has no target (digits stay numbers);
    #    both references keep the digit inside the word and then restore the whole word.
    if method == "V" and re.search(r"[0-9]", sentype):
        return "style", "vni-digit-ends-word"

    # 3. SenType composed a dictionary syllable but the reference restored raw keys.
    if raw_ref and not raw_sentype and all_dict(sentype):
        return "reference-quirk", "stricter-than-dictionary"

    # 4. The reference composed something the dictionary does not list (ka, ce, coa, toneless
    #    stop-final bêt...) while SenType restored raw: SenType follows the dictionary.
    if raw_sentype and not raw_ref and not all_dict(refout):
        return "style", "dictionary-strict"

    # 5. Free marking of the stroke: SenType needs "dd" right after the initial (no "did" -> "đi").
    if method == "T" and raw_sentype and "đ" in refout.lower() and keys.lower().startswith("d"):
        return "style", "stroke-needs-dd"

    return "unclassified", "-"
