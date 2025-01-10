#!/usr/bin/env python3

import re

rename_and = re.compile(r"x(\d\d) AND y(\d\d) -> (?!z)(\w+)")
swap_xor2 = re.compile(r"(...) XOR XOR(\d\d) -> z(\d\d)")
swap_or = re.compile(r"(...) OR AND(\d\d) -> (...)")
rename_or = re.compile(r"AND(\d\d) OR (...) -> (?!z)(...)")


def read_lines():
    with open("test/d24.txt", "r", encoding="utf-8") as infile:
        return [line.strip() for line in infile.readlines()]


def apply_renames(lines, renames) -> list[str]:
    new_lines = []
    for line in lines:
        for src, dst in renames.items():
            line = line.replace(src, dst)
        new_lines.append(line)
    return new_lines


def apply_first(lines, renames) -> list[str]:
    for src, dst in renames.items():
        lines = [
            line.replace(f"-> {src}", "-> XXX")
            .replace(f"-> {dst}", f"-> {src}")
            .replace("-> XXX", f"-> {dst}")
            for line in lines
        ]
    return lines


def apply1(lines) -> list[str]:
    swap_xor = re.compile(r"(y\d\d) XOR (x\d\d) -> (\w+)")
    new_lines = []
    for line in lines:
        if m := swap_xor.match(line):
            line = f"{m.group(2)} XOR {m.group(1)} -> {m.group(3)}"
        new_lines.append(line)
    return new_lines


def apply2(lines) -> list[str]:
    swap_and = re.compile(r"(y\d\d) AND (x\d\d) -> (\w+)")
    new_lines = []
    for line in lines:
        if m := swap_and.match(line):
            line = f"{m.group(2)} AND {m.group(1)} -> {m.group(3)}"
        new_lines.append(line)
    return new_lines


def apply3(lines, renames) -> list[str]:
    rename_xor = re.compile(r"x(\d\d) XOR y(\d\d) -> (?!z)(\w+)")
    new_lines = []
    for line in lines:
        if m := rename_xor.match(line):
            line = f"x{m.group(1)} XOR y{m.group(2)} -> XOR{m.group(1)}"
            renames[m.group(3)] = f"XOR{m.group(1)}"
        new_lines.append(line)
    return new_lines


def apply4(lines, renames) -> list[str]:
    new_lines = []
    for line in lines:
        if m := rename_and.match(line):
            line = f"x{m.group(1)} AND y{m.group(2)} -> AND{m.group(1)}"
            renames[m.group(3)] = f"AND{m.group(1)}"
        new_lines.append(line)
    return new_lines


def apply5(lines) -> list[str]:
    new_lines = []
    for line in lines:
        if m := swap_xor2.match(line):
            line = f"XOR{m.group(2)} XOR {m.group(1)} -> z{m.group(3)}"
        new_lines.append(line)
    return new_lines


def apply6(lines) -> list[str]:
    new_lines = []
    for line in lines:
        if m := swap_or.match(line):
            line = f"AND{m.group(2)} OR {m.group(1)} -> {m.group(3)}"
        new_lines.append(line)
    return new_lines


def apply7(lines, renames) -> list[str]:
    new_lines = []
    for line in lines:
        if m := rename_or.match(line):
            line = f"AND{m.group(1)} OR {m.group(2)} -> C{m.group(1)}"
            renames[m.group(3)] = f"C{m.group(1)}"
        new_lines.append(line)
    return new_lines


def apply8(lines) -> list[str]:
    swap_and2 = re.compile(r"XOR(\d\d) AND C(\d\d) -> (\w+)")
    new_lines = []
    for line in lines:
        if m := swap_and2.match(line):
            line = f"C{m.group(2)} AND XOR{m.group(1)} -> {m.group(3)}"
        new_lines.append(line)
    return new_lines


def apply9(lines, renames) -> list[str]:
    rename = re.compile(r"C(\d\d) AND XOR(\d\d) -> (?!z)(...)")
    new_lines = []
    for line in lines:
        if m := rename.match(line):
            line = f"C{m.group(1)} AND XOR{m.group(2)} -> AND_{m.group(2)}"
            renames[m.group(3)] = f"AND_{m.group(2)}"
        new_lines.append(line)
    return new_lines


def apply10(lines, renames) -> list[str]:
    rename = re.compile(r"XOR01 AND AND00 -> (?!z)(...)")
    new_lines = []
    for line in lines:
        if m := rename.match(line):
            line = f"AND00 AND XOR01 -> AND_01"
            renames[m.group(1)] = f"AND_01"
        new_lines.append(line)
    return new_lines


renames = dict()
lines = read_lines()
lines = apply_first(lines, {"dhq": "z18", "kfp": "hbs", "pdg": "z22", "jcp": "z27"})

lines = apply1(lines)
lines = apply2(lines)
lines = apply3(lines, renames)
lines = apply_renames(lines, renames)
lines = apply4(lines, renames)
lines = apply_renames(lines, renames)
lines = apply5(lines)
lines = apply6(lines)
lines = apply7(lines, renames)
lines = apply_renames(lines, renames)
lines = apply8(lines)
lines = apply9(lines, renames)
lines = apply_renames(lines, renames)
lines = apply10(lines, renames)
lines = apply_renames(lines, renames)

with open("test/d24.txt.new", "w", encoding="utf-8") as outfile:
    for line in lines:
        print(line, file=outfile)

gates = []
for k, v in {"dhq": "z18", "kfp": "hbs", "pdg": "z22", "jcp": "z27"}.items():
    gates.extend((k, v))

gates.sort()
print(",".join(gates))
