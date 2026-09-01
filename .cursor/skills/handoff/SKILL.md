---
name: handoff
description: Writes timestamped session handoff markdown for the next Cursor chat. Use when ending a session, starting fresh chat, @handoff, session continuity, or agent handoff.
disable-model-invocation: true
---

# Handoff

Persist **session state on disk** so the next chat does not rely on memory or long paste. Complements Cursor `@Past Chats` ([agent best practices](https://cursor.com/blog/agent-best-practices)) and `.cursor/plans/` (Plan Mode).

## When to run

- User says `@handoff`, "write handoff", or "end session"
- Before starting a **new chat** after long/noisy thread
- After a lesson block (`@teach-me`) or a multi-step procedure
- Between reprogram steps (0-5) when the thread is long: `@handoff` then continue with the **Next @ skill** in a fresh chat

Start a **new conversation** when scope changes or agent quality drops; load this skill in the new chat.

## Where files go

| Path | Role |
| :--- | :--- |
| `.cursor/skills/handoff/sessions/yyyy-mm-ddTHHmm-<slug>.md` | Immutable snapshot |
| `.cursor/skills/handoff/sessions/latest.md` | Overwritten; open with `@handoff` |
| [HANDOFF-FORMAT.md](HANDOFF-FORMAT.md) | Template |

`sessions/` is gitignored (may contain hostnames or URLs). Skill + format stay tracked.

## Write workflow (agent)

1. Gather from chat: mission, done, **one** next step, todos, blockers, branch, environment, sources used (paths or URLs). For reprogram sessions: device folder slug, serial port, step completed, **Next @ skill** (e.g. `@2-dump-firmware`), blocker.
2. Slug from mission (lowercase hyphenated topic). Time: ISO `yyyy-mm-ddTHHmm` (local).
3. Write **both** timestamped file and `sessions/latest.md` using [HANDOFF-FORMAT.md](HANDOFF-FORMAT.md).
4. If Plan Mode file exists in `.cursor/plans/`, link it under **Pointers** (do not duplicate the plan).
5. If `@teach-me`: link `teaching/MISSION.md` (or the project's teaching path) under **Pointers**.
6. If glossary terms changed: link `docs/CONTEXT.md` under **Pointers** (update that file, do not copy terms into handoff).
7. Confirm paths written; paste **Fresh-chat opener** for user to copy.

## Read workflow (new chat)

1. Read `sessions/latest.md` first; read timestamped file only if user names it.
2. Read pointers (`docs/`, `docs/CONTEXT.md`, plans, teaching) before acting. Do not treat `notes/` as writable.
3. Honor **Mode** in opener: `teach` = user runs commands; `apply` = agent may edit/run.

## Layering (do not duplicate)

| Layer | Use for |
| :--- | :--- |
| **Handoff** | Session snapshot, next step, blockers, todos |
| `.cursor/plans/` | Feature implementation plan (Plan Mode) |
| `docs/` (incl. `docs/CONTEXT.md`) | Durable facts and glossary (writable by agents per project map) |
| `notes/` | Obsidian SoT (read-only for agents) |
| `teaching/` | Learning mission and preferences |
| Rules / skills | Stable how-to |

## Environment notes

- Label **which machine** when commands differ across hosts.
- Never store tokens, passwords, or credentials in handoff.

## Out of scope

- Replacing `docs/` (incl. `docs/CONTEXT.md`) or teaching files as SoT for durable facts
- Writing or editing `notes/` (Obsidian only)
- Auto-commit handoff files (user commits if they want history in git)
