with open('spritesheet_rules.mk', 'r') as f:
    content = f.read()
fixed = content.replace('        $(GFX)', '\t$(GFX)')
with open('spritesheet_rules.mk', 'w') as f:
    f.write(fixed)
print('done')
