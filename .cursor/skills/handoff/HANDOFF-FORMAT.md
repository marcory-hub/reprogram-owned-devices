# Handoff file format

Filename: `yyyy-mm-ddTHHmm-<slug>.md` (ISO 8601 local time, slug = lowercase hyphenated topic).

Also write/overwrite: `sessions/latest.md` (same content; entry point for `@handoff`).

## Template

~~~~markdown
# Handoff: <title>

**Created:** yyyy-mm-ddTHHmm
**Status:** active | blocked | done
**Branch:** <git branch or n/a>
**Mission:** <one sentence goal>

## Done this session

- [x] item

## Next step (single)

One executable step for the next agent or chat.

## Todo

- [ ] item
- [ ] item

## Blockers / failed attempts

- What was tried and why it failed (prevents repeat loops)

## Environment (if relevant)

| Key | Value |
| :--- | :--- |
| Machine | <host or role label> |
| Services | <status, ports; no secrets> |

## Reprogram session (when applicable)

| Key | Value |
| :--- | :--- |
| Device folder | `docs/<modelnumber>-<short-name>/` slug |
| Serial port | e.g. `/dev/cu.usbserial-*` or `/dev/cu.wchusbserial*` |
| Step completed | last finished step (0-5) or bug repro |
| Next @ skill | e.g. `@2-dump-firmware` |
| Blocker | what failed or what waits on the user |

Durable facts stay in `docs/<device>/`. Handoff holds session state only.

## Pointers

- Plan: `.cursor/plans/<file>.md` (if any)
- Teaching: `teaching/MISSION.md` (if any)
- CONTEXT: `docs/CONTEXT.md`
- Docs: `docs/<device>/` (operator procedures)
- Notes: `notes/` (read-only; Obsidian)
- Artifacts: paths only (no secrets)
- Sources: paths or URLs used this session (provenance; not every chat turn)

## Fresh-chat opener

Reprogram demo (typical):

```text
@handoff
Continue from latest handoff. Then invoke the Next @ skill from the handoff with port and folder from that file.
Mode: teach | apply.
```

Generic:

```text
@handoff
Continue from latest handoff. Mode: teach | apply.
```

## Rules

- No API keys, tokens, or passwords in handoff files.
- Durable facts belong in `docs/` (glossary in `docs/CONTEXT.md`); handoff is session state only.
- One **Next step** only; queue extras under **Todo**.
~~~~
