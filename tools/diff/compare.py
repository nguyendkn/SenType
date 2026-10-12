#!/usr/bin/env python3
"""Compare SenType with UniKey and Bamboo outputs; classify every difference.

Categories (see classify.py for the rules):
  sentype-bug      SenType output is wrong against a Vietnamese reference answer
  style            legitimate design difference (documented in docs/testing.md)
  reference-quirk  the reference engine misbehaves (output is not a sane answer)
  unclassified     needs a human look (must be 0 before a release)
Usage: compare.py <dir with inputs.tsv sentype.txt unikey.txt bamboo.txt> <vi-syllables.txt> <non-native-syllables.txt>
"""
import collections
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import classify as cl  # noqa: E402
from classify import classify  # noqa: E402


def read(path):
    with open(path, encoding="utf-8", errors="replace") as f:
        return f.read().split("\n")[:-1]


def main(d, dictionary, exclusions):
    # Ground truth = dictionary minus the hand-reviewed non-native entries (same as the core test).
    cl.DICT.update(set(read(dictionary)) - {l for l in read(exclusions) if l and l[0] != "#"})
    inputs = [l.split("\t") for l in read(os.path.join(d, "inputs.tsv"))]
    sentype = read(os.path.join(d, "sentype.txt"))
    refs = {n: read(os.path.join(d, n + ".txt")) for n in ("unikey", "bamboo")}
    assert len(inputs) == len(sentype) == len(refs["unikey"]) == len(refs["bamboo"]), "length mismatch"

    table = collections.defaultdict(collections.Counter)  # (ref, label) -> category counts
    samples = collections.defaultdict(list)
    for name, out in refs.items():
        with open(os.path.join(d, "diff-" + name + ".tsv"), "w", encoding="utf-8") as f:
            for (label, method, keys), s, r, u, b in zip(inputs, sentype, out, refs["unikey"], refs["bamboo"]):
                if s == r:
                    table[(name, label)]["same"] += 1
                    continue
                other = b if name == "unikey" else u
                cat = classify(name, method, keys, s, r, other)
                table[(name, label)][cat] += 1
                f.write("\t".join([cat, label, method, keys, "sentype=" + s, name + "=" + r]) + "\n")
                if len(samples[(name, cat)]) < 12:
                    samples[(name, cat)].append((keys, s, r))

    cats = ["same", "style", "reference-quirk", "sentype-bug", "unclassified"]
    print("%-8s %-5s %8s" % ("ref", "set", "total") + "".join("%16s" % c for c in cats))
    for (name, label) in sorted(table):
        row = table[(name, label)]
        print("%-8s %-5s %8d" % (name, label, sum(row.values())) + "".join("%16d" % row[c] for c in cats))
    print()
    for (name, cat), lst in sorted(samples.items()):
        print("[%s / %s] samples (keys -> sentype | ref):" % (name, cat))
        for keys, s, r in lst:
            print("   %-22s %-16s | %s" % (keys, s, r))


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2], sys.argv[3])
