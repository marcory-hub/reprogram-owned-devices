---
name: cursor-unignore
description: Sync .cursorignore negation patterns with .gitignore by discovering gitignored paths the codebase actually references. Use when updating .gitignore, fixing Cursor access, unignore, cursorignore, or @cursor-unignore.
---

# Cursor unignore

Cursor honors `.gitignore` and built-in ignores. Re-include agent-needed paths with `!` in `.cursorignore`. [Docs](https://cursor.com/docs/reference/ignore-file).

**Every run:** derive candidates from this repo; never copy patterns from memory or other projects.

## Workflow

1. **Read** root `.gitignore`, `.cursorignore`, and any nested `.gitignore` files.
2. **Discover demand:** search the codebase for paths/rules/skills/README/scripts that reference locations under gitignore:
   - Grep for directory names from `.gitignore` and phrases like `gitignored`, `cursorignore`, `Read tool`.
   - List on-disk children of ignored parents (e.g. `notes/`, `data/raw/`).
   - Prefer the **smallest** subpath that satisfies references (e.g. one vault subfolder, not all of `notes/`).
3. **Filter out** paths that must stay blocked:

| Block | Examples |
| :--- | :--- |
| Secrets | `.env`, `.env.*`, `*credentials*`, `*secret*`, `*.pem`, `*.key` |
| Dependencies | `.venv/`, `node_modules/`, `vendor/` |
| Caches / tooling | `__pycache__/`, `.*_cache/`, `.pytest_cache/`, `.mypy_cache/` |
| OS / IDE / build | `.DS_Store`, `.vscode/`, `dist/`, `build/`, `*.o` |
| Large binaries | `*.pdf`, archives, media (unless an in-repo skill/rule **explicitly** requires Agent Read on that path) |

4. **Diff** filtered candidates against existing `.cursorignore` negations; add only missing entries.
5. **Write patterns:** parent before child; directory patterns end with `/`:

```gitignore
parent/*
!parent/needed/
!parent/needed/**
```

For paths blocked by Cursor **default** ignores (images, PDFs), add explicit `!path/**/*.ext` per extension.

6. **Verify:** `git check-ignore -v <sample-file>` then Agent Read on one sample per new block.
7. **Format:** one short comment line per pattern block only; no file header, syntax notes, or policy tables.

Example (substitute the smallest discovered subpath for `design`):

```gitignore
# notes/design
notes/*
!notes/design/
!notes/design/**
```

## Limits

- `.cursorignore` does not block terminal/MCP reads.
- Negation fails if a parent dir is excluded with `*`; un-ignore each level explicitly.
- When demand is unclear, ask the user before un-ignoring broad trees or binaries.
