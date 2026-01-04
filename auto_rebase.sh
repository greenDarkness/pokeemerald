#!/bin/bash
# Auto-continue rebase, resolving binary conflicts with --theirs
# Auto-resolves text conflicts where HEAD side is empty (just additions from theirs)
# Stops on other text conflicts for manual review

cd /mnt/d/projects/pokeemerald

while true; do
    # Check if rebase is in progress
    if [ ! -d .git/rebase-merge ] && [ ! -d .git/rebase-apply ]; then
        echo "REBASE COMPLETE!"
        break
    fi

    # Show progress
    if [ -f .git/rebase-merge/msgnum ]; then
        current=$(cat .git/rebase-merge/msgnum)
        total=$(cat .git/rebase-merge/end)
        echo "Progress: $current / $total"
    fi

    # Get conflicting files
    conflicts=$(git diff --name-only --diff-filter=U 2>/dev/null)
    
    if [ -z "$conflicts" ]; then
        # No conflicts - might be empty commit, try to continue or skip
        result=$(GIT_EDITOR=true git rebase --continue 2>&1)
        if echo "$result" | grep -q "nothing to commit"; then
            git rebase --skip 2>&1 | tail -3
        else
            echo "$result" | tail -5
        fi
        continue
    fi

    has_manual_conflict=false
    
    for f in $conflicts; do
        if file "$f" 2>/dev/null | grep -qE "text|ASCII|UTF|script|source|empty|JSON"; then
            # Text file - check conflict type
            if grep -q "<<<<<<< HEAD" "$f" 2>/dev/null; then
                # Check if HEAD side is empty (just additions from theirs)
                # Pattern: <<<<<<< HEAD\n=======\n...content...\n>>>>>>>
                if python3 -c "
import re, sys
content = open('$f', 'r').read()
# Find all conflict blocks where HEAD side is empty
pattern = r'<<<<<<< HEAD\n=======\n(.*?)\n>>>>>>> [^\n]+'
matches = re.findall(pattern, content, re.DOTALL)
conflicts_found = len(re.findall(r'<<<<<<< HEAD', content))
if matches and len(matches) == conflicts_found:
    # All conflicts have empty HEAD - auto-resolve with theirs
    resolved = re.sub(r'<<<<<<< HEAD\n=======\n(.*?)\n>>>>>>> [^\n]+', r'\1', content, flags=re.DOTALL)
    open('$f', 'w').write(resolved)
    sys.exit(0)
else:
    sys.exit(1)
" 2>/dev/null; then
                    echo "  Auto-resolved (theirs added): $f"
                    git add "$f"
                else
                    has_manual_conflict=true
                    echo "TEXT CONFLICT in: $f"
                fi
            else
                git checkout --theirs "$f" 2>/dev/null
                git add "$f"
            fi
        else
            # Binary file - take theirs (bridge version)
            git checkout --theirs "$f" 2>/dev/null
            git add "$f"
        fi
    done

    if [ "$has_manual_conflict" = true ]; then
        echo "STOPPED: Manual review needed. Remaining conflicts:"
        git diff --name-only --diff-filter=U 2>/dev/null
        break
    fi

    # All conflicts resolved, continue
    GIT_EDITOR=true git rebase --continue 2>&1 | tail -3
done
