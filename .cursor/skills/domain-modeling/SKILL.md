---
name: domain-modeling
description: Build and sharpen a project's domain model. Use when discussing codebase terminology or writing or editing docs/CONTEXT.md or docs/<device>/.
---

# Domain Modeling

Actively build and sharpen the project's domain model as you design. Challenge terms, stress-test scenarios, and write the glossary the moment terms crystallise. (Reading the glossary for vocabulary is not this skill. This skill is for when you are changing the model.)

**This repo:** glossary at `docs/CONTEXT.md`; device operator facts under `docs/<device>/` (see project map Paths). Use `docs/CONTEXT.md` below wherever a template says root `CONTEXT.md`.

## File structure

```
/
├── docs/
│   ├── CONTEXT.md           ← glossary (this repo)
│   └── <device>/            ← operator procedures per board
└── firmware/
    └── <device>/
```

If a `CONTEXT-MAP.md` exists at the root, the repo has multiple glossaries. Read it to find paths.

Create files lazily: glossary in `docs/CONTEXT.md` when the first term is resolved; device procedures under `docs/<device>/` when a board is in scope.

## During the session

### Challenge against the glossary

When the user uses a term that conflicts with `docs/CONTEXT.md`, call it out. "Your glossary defines X as …, but you seem to mean Y. Which is it?"

### Sharpen fuzzy language

When the user uses vague or overloaded terms, propose a precise canonical term.

### Discuss concrete scenarios

Stress-test domain relationships with specific scenarios. Force precise boundaries between concepts.

### Cross-reference with code

When the user states how something works, check whether the code agrees. Surface contradictions.

### Update CONTEXT.md inline

When a term is resolved, update `docs/CONTEXT.md` right there. Use [CONTEXT-FORMAT.md](./CONTEXT-FORMAT.md).

`docs/CONTEXT.md` is a glossary only. No implementation details, specs, or scratch notes.

### Device facts

Board-specific procedures, GPIO maps, backup steps, and flash paths belong in `docs/<device>/`, not in `docs/CONTEXT.md` or handoff. Link from CONTEXT when a term points at a device tree.
