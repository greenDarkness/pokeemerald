#!/usr/bin/env python3
r"""
Pokemon Emerald String Validator and Auto-Fixer

Validates .inc files containing game text strings to ensure:
1. Valid escape sequences (\n, \p, \l, $, etc.)
2. Line lengths don't exceed 34 visible characters (game text box limit)
3. Proper .string directive formatting
4. Matching/balanced quotes
5. Proper $ terminator on final strings of each text block
6. No broken or malformed string lines

Usage:
    python validate_strings.py <file.inc>          # Validate and show issues
    python validate_strings.py <file.inc> --fix    # Auto-fix issues in place

Valid escape sequences for pokeemerald:
    \l  - scroll up window text (FA)
    \p  - new paragraph (FB)
    \n  - new line (FE)
    $   - end of string marker (required at end of each text block)
    
Special placeholders (not counted toward line length):
    {STR_VAR_1}, {STR_VAR_2}, {STR_VAR_3}, {PLAYER}, {RIVAL}, etc.
"""

import re
import sys
import argparse
from pathlib import Path
from dataclasses import dataclass, field
from typing import Optional, List, Tuple

# Valid escape sequences in pokeemerald text
VALID_ESCAPES = {'n', 'p', 'l'}

# Estimated widths for common placeholders (conservative estimates)
PLACEHOLDER_WIDTHS = {
    'STR_VAR_1': 10,
    'STR_VAR_2': 10,
    'STR_VAR_3': 10,
    'PLAYER': 7,
    'RIVAL': 5,
}

# Max visible characters per line in GBA text box
# Note: Pokemon Emerald uses variable-width fonts (3-8 pixels per character)
# The text box is 216 pixels wide. Vanilla game has strings up to 40 chars
# (excluding $ terminator and {} placeholders which are variable).
MAX_LINE_LENGTH = 40


@dataclass
class Issue:
    line_num: int
    col: int
    issue_type: str
    message: str
    original: str
    suggested: Optional[str] = None
    fixable: bool = True


@dataclass
class TextBlock:
    """Represents a labeled text block (label + string lines)."""
    label: str
    label_line: int
    string_lines: List[Tuple[int, str]] = field(default_factory=list)  # (line_num, content)


def get_visible_length(text: str) -> int:
    """
    Calculate the visible length of text, excluding escape sequences
    and estimating placeholder widths.
    """
    # Remove escape sequences (\n, \p, \l)
    visible = re.sub(r'\\[npl]', '', text)
    
    # Handle placeholders - estimate their width
    for placeholder, width in PLACEHOLDER_WIDTHS.items():
        pattern = r'\{' + placeholder + r'(?:\[[^\]]*\])?\}'
        visible = re.sub(pattern, 'X' * width, visible)
    
    # Handle other curly brace placeholders generically
    visible = re.sub(r'\{[^}]+\}', 'XXXXXXXX', visible)
    
    # Remove the trailing $ if present
    if visible.endswith('$'):
        visible = visible[:-1]
    
    return len(visible)


def extract_string_content(line: str) -> Optional[str]:
    """Extract the content between quotes from a .string directive."""
    match = re.search(r'\.string\s+"([^"]*)"', line)
    if match:
        return match.group(1)
    return None


def find_invalid_escapes(text: str) -> List[Tuple[int, str]]:
    """Find invalid escape sequences and return their positions."""
    invalid = []
    i = 0
    while i < len(text):
        if text[i] == '\\':
            if i + 1 < len(text):
                next_char = text[i + 1]
                if next_char not in VALID_ESCAPES:
                    invalid.append((i, next_char))
            else:
                # Backslash at end of string with no following char
                invalid.append((i, ''))
            i += 2
        else:
            i += 1
    return invalid


def get_line_segments(text: str) -> List[str]:
    r"""
    Split text into segments by line breaks (\n, \l, \p).
    Returns list of segments that should each fit within MAX_LINE_LENGTH.
    """
    segments = re.split(r'\\[npl]', text)
    if segments and segments[-1].endswith('$'):
        segments[-1] = segments[-1][:-1]
    return segments


def suggest_escape_fix(invalid_char: str) -> str:
    """Suggest the most likely intended escape sequence."""
    suggestions = {
        '1': 'p',  # \1 likely meant \p (looks similar)
        '2': 'p',
        '3': 'p',
        '0': 'n',
        'N': 'n',
        'P': 'p',
        'L': 'l',
        '': 'p',  # Trailing backslash -> \p
    }
    return suggestions.get(invalid_char, 'p')


def check_quote_balance(line: str) -> Tuple[bool, str]:
    """
    Check if quotes are balanced in a line.
    Returns (is_valid, error_message).
    """
    # Count quotes outside of escaped contexts
    in_string = False
    quote_count = 0
    
    i = 0
    while i < len(line):
        if line[i] == '"':
            # Check if it's escaped
            if i > 0 and line[i-1] == '\\':
                i += 1
                continue
            quote_count += 1
            in_string = not in_string
        i += 1
    
    if quote_count % 2 != 0:
        return False, "Unbalanced quotes (odd number of quote marks)"
    
    return True, ""


def check_string_directive(line: str) -> Tuple[bool, str, Optional[str]]:
    """
    Check if a line that looks like it should be a .string directive is valid.
    Returns (is_valid, error_message, suggested_fix).
    """
    stripped = line.strip()
    
    # Skip empty lines, comments, and labels
    if not stripped or stripped.startswith('@') or stripped.endswith('::'):
        return True, "", None
    
    # Check for malformed .string directive
    if '.string' in stripped.lower():
        # Should match: .string "content"
        if not re.match(r'\s*\.string\s+"[^"]*"\s*(@.*)?$', line):
            # Try to identify the issue
            
            # Missing opening quote
            if re.match(r'\s*\.string\s+[^"]+', line) and '"' not in line:
                return False, "Missing quotes around string content", None
            
            # Missing closing quote
            if line.count('"') == 1:
                content_match = re.search(r'\.string\s+"([^"]*)', line)
                if content_match:
                    content = content_match.group(1)
                    return False, "Missing closing quote", f'\t.string "{content}"'
            
            # Has content but missing .string
            return False, "Malformed .string directive", None
    
    # Check if line has quoted content but no .string
    if '"' in stripped and not stripped.startswith('.') and not stripped.endswith('::'):
        # Might be a string without the .string directive
        quote_match = re.search(r'"([^"]*)"', stripped)
        if quote_match:
            content = quote_match.group(1)
            return False, "Quoted text without .string directive", f'\t.string "{content}"'
    
    return True, "", None


def parse_text_blocks(lines: List[str]) -> List[TextBlock]:
    """Parse the file into text blocks (label + strings)."""
    blocks = []
    current_block = None
    
    for line_num, line in enumerate(lines, 1):
        stripped = line.strip()
        
        # Check for label
        if stripped.endswith('::'):
            if current_block and current_block.string_lines:
                blocks.append(current_block)
            label = stripped[:-2]
            current_block = TextBlock(label=label, label_line=line_num)
        
        # Check for .string line
        elif current_block and '.string' in line.lower():
            content = extract_string_content(line)
            if content is not None:
                current_block.string_lines.append((line_num, content))
    
    if current_block and current_block.string_lines:
        blocks.append(current_block)
    
    return blocks


def validate_file(filepath: Path) -> List[Issue]:
    """Validate a .inc file and return list of issues found."""
    issues = []
    
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    # Parse into text blocks for $ terminator checking
    blocks = parse_text_blocks(lines)
    
    for line_num, line in enumerate(lines, 1):
        stripped = line.strip()
        
        # Skip empty lines, comments, and labels
        if not stripped or stripped.startswith('@') or stripped.endswith('::'):
            continue
        
        # Check quote balance
        quote_valid, quote_error = check_quote_balance(line)
        if not quote_valid:
            issues.append(Issue(
                line_num=line_num,
                col=0,
                issue_type='quote_error',
                message=quote_error,
                original=line.rstrip(),
                fixable=False
            ))
            continue
        
        # Check .string directive format
        directive_valid, directive_error, suggested = check_string_directive(line)
        if not directive_valid:
            issues.append(Issue(
                line_num=line_num,
                col=0,
                issue_type='directive_error',
                message=directive_error,
                original=line.rstrip(),
                suggested=suggested,
                fixable=suggested is not None
            ))
            continue
        
        # Process valid .string lines
        content = extract_string_content(line)
        if content is None:
            continue
        
        # Check for invalid escape sequences
        invalid_escapes = find_invalid_escapes(content)
        for col, bad_char in invalid_escapes:
            suggested = suggest_escape_fix(bad_char)
            display_char = bad_char if bad_char else '(end of line)'
            issues.append(Issue(
                line_num=line_num,
                col=col,
                issue_type='invalid_escape',
                message=f"Invalid escape sequence '\\{display_char}'",
                original=f'\\{bad_char}',
                suggested=f'\\{suggested}'
            ))
        
        # Check line segment lengths
        segments = get_line_segments(content)
        for segment in segments:
            visible_len = get_visible_length(segment)
            if visible_len > MAX_LINE_LENGTH and segment:
                truncated = segment[:40] + '...' if len(segment) > 40 else segment
                issues.append(Issue(
                    line_num=line_num,
                    col=0,
                    issue_type='line_too_long',
                    message=f"Text segment too long ({visible_len} chars, max {MAX_LINE_LENGTH}): '{truncated}'",
                    original=segment,
                    fixable=False
                ))
    
    # Check for missing $ terminators at end of text blocks
    for block in blocks:
        if block.string_lines:
            last_line_num, last_content = block.string_lines[-1]
            if not last_content.endswith('$'):
                issues.append(Issue(
                    line_num=last_line_num,
                    col=len(last_content),
                    issue_type='missing_terminator',
                    message=f"Text block '{block.label}' missing $ terminator at end",
                    original=last_content,
                    suggested=last_content + '$'
                ))
    
    return issues


def fix_file(filepath: Path, issues: List[Issue]) -> int:
    """Apply fixes to the file. Returns number of fixes applied."""
    with open(filepath, 'r', encoding='utf-8') as f:
        lines = f.readlines()
    
    fixes_applied = 0
    
    # Group issues by line for efficient processing
    issues_by_line = {}
    for issue in issues:
        if issue.fixable and issue.suggested:
            if issue.line_num not in issues_by_line:
                issues_by_line[issue.line_num] = []
            issues_by_line[issue.line_num].append(issue)
    
    # Process each line
    for line_num in sorted(issues_by_line.keys()):
        line_issues = issues_by_line[line_num]
        line = lines[line_num - 1]
        
        for issue in line_issues:
            if issue.issue_type == 'invalid_escape':
                old = issue.original
                new = issue.suggested
                if old in line:
                    line = line.replace(old, new, 1)
                    fixes_applied += 1
                    print(f"  Line {line_num}: Fixed escape {old} -> {new}")
            
            elif issue.issue_type == 'missing_terminator':
                # Add $ before the closing quote
                match = re.search(r'"([^"]*)"', line)
                if match and not match.group(1).endswith('$'):
                    old_content = match.group(1)
                    new_content = old_content + '$'
                    line = line.replace(f'"{old_content}"', f'"{new_content}"', 1)
                    fixes_applied += 1
                    print(f"  Line {line_num}: Added missing $ terminator")
            
            elif issue.issue_type == 'directive_error' and issue.suggested:
                # Replace the entire line with the suggested fix
                line = issue.suggested + '\n'
                fixes_applied += 1
                print(f"  Line {line_num}: Fixed directive -> {issue.suggested.strip()}")
        
        lines[line_num - 1] = line
    
    with open(filepath, 'w', encoding='utf-8') as f:
        f.writelines(lines)
    
    return fixes_applied


def print_issues(issues: List[Issue], filepath: Path):
    """Print issues in a readable format."""
    if not issues:
        print(f"✓ {filepath}: No issues found!")
        return
    
    print(f"\n{'='*60}")
    print(f"Issues in {filepath}")
    print(f"{'='*60}\n")
    
    # Group by issue type
    issue_types = {
        'invalid_escape': ('INVALID ESCAPE SEQUENCES', True),
        'missing_terminator': ('MISSING $ TERMINATORS', True),
        'quote_error': ('QUOTE ERRORS', False),
        'directive_error': ('DIRECTIVE ERRORS', True),
        'line_too_long': ('LINE LENGTH ISSUES', False),
    }
    
    for issue_type, (title, is_fixable) in issue_types.items():
        type_issues = [i for i in issues if i.issue_type == issue_type]
        if type_issues:
            fix_note = " (auto-fixable)" if is_fixable else " (requires manual fix)"
            print(f"{title}{fix_note}:")
            print("-" * 50)
            for issue in type_issues:
                hint = f" -> {issue.suggested}" if issue.suggested else ""
                print(f"  Line {issue.line_num}: {issue.message}{hint}")
            print()
    
    # Summary
    fixable = sum(1 for i in issues if i.fixable and i.suggested)
    manual = len(issues) - fixable
    print(f"Total: {len(issues)} issues ({fixable} auto-fixable, {manual} manual)")


def main():
    parser = argparse.ArgumentParser(
        description='Validate and fix Pokemon Emerald .inc text files'
    )
    parser.add_argument('file', type=Path, help='Path to .inc file to validate')
    parser.add_argument('--fix', action='store_true', 
                        help='Auto-fix issues where possible')
    parser.add_argument('--quiet', '-q', action='store_true',
                        help='Only show errors, not success messages')
    
    args = parser.parse_args()
    
    if not args.file.exists():
        print(f"Error: File not found: {args.file}", file=sys.stderr)
        sys.exit(1)
    
    if not args.file.suffix == '.inc':
        print(f"Warning: File does not have .inc extension: {args.file}")
    
    # Validate
    issues = validate_file(args.file)
    
    if args.fix:
        if issues:
            fixable_issues = [i for i in issues if i.fixable and i.suggested]
            if fixable_issues:
                print(f"Fixing {len(fixable_issues)} issues in {args.file}...")
                fixes = fix_file(args.file, issues)
                print(f"Applied {fixes} fixes.\n")
            
            # Re-validate to show remaining issues
            remaining = validate_file(args.file)
            if remaining:
                print_issues(remaining, args.file)
                print("\nNote: Some issues require manual fixing.")
            else:
                print(f"✓ All issues fixed in {args.file}!")
        else:
            if not args.quiet:
                print(f"✓ {args.file}: No issues to fix!")
    else:
        print_issues(issues, args.file)
        
        if issues:
            fixable = sum(1 for i in issues if i.fixable and i.suggested)
            if fixable > 0:
                print(f"\nRun with --fix to auto-fix {fixable} issues.")
            sys.exit(1)


if __name__ == '__main__':
    main()
