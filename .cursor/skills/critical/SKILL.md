---
name: critical
description: Argument analysis, reference audit, and statistics check (Lavin + Van Cleave + science methods CC). Use when @critical, @fact-check, critical thinking, evaluate argument, fact-check claims, peer review, references, statistics, study design, reasoning audit, klopt dit, fallacy, bias, or draft audit.
---

# Critical thinking

Portable argument analysis, reference audit, and statistical sanity-check. Knowledge lives in `.cursor/skills/critical/knowledge/`. Activation via `@critical`, `@fact-check`, or description keywords. Never always-on.

**Built from:** Lavin (*Thinking Well*) and Van Cleave (*Introduction to Logic and Critical Thinking*), CC BY 4.0; empirical audit methods distilled from Jarrard (*Scientific Methods*, CC BY-SA 3.0). Source files are not read at runtime.

## When to use

**Use for:** judging pasted prose and citations: argument structure, whether facts are supported, reference tier (peer-reviewed vs not), study design fit, statistical claims.

**Pairing:** When claims must be verified by reading project notes or references, open those paths with the Read tool (or the project's notes skill if one exists). `@critical` supplies the audit framework on what you paste; it does not open project PDFs by default.

**Not in scope:** formal logic proofs, prose style while drafting, editor read-through of article drafts, domain synthesis without sources.

## 1. Classify task

| Type | Signals | Action |
| :--- | :--- | :--- |
| Reasoning | weak reasoning, assumptions, structure | Reasoning pass |
| Evidence | references, peer review, true?, validated | Science + fact-check pass |
| Statistics | p-value, sample, correlation, causation | Science pass |
| Full | draft with citations + argument | Science + fact-check, then reasoning |

Default: Full when user pastes draft with references; Fact-check for `@fact-check`; Evaluate for logic-only `@critical`.

## 2. Retrieve (mandatory)

1. **Read** `knowledge/index.md`
2. **Read** at most 2 files index points to (`analyze.md`, `evaluate.md`, `science.md`)
3. Do not Read the whole `knowledge/` tree or external textbooks at runtime

## 3. Deliver results

Output the audit directly. No skill meta unless asked.

## 4. Reasoning pass

Use `analyze.md` + `evaluate.md` when loaded: standard form, hidden premises, validity/strength, fallacies, IBE, confidence.

## 5. Science and fact-check pass

Use `science.md` when loaded:

- Label each empirical claim: established / supported / preliminary / unsupported / overstated / wrong / [needs verification]
- Label each reference: tier A-F and citation issues (misquote, overreach, weak tier)
- Flag statistical method problems per `science.md` table
- Group **Fix** / **Soften** / **Check**
- Do not invent sources, DOIs, or results

## 6. Output modes

| Mode | Trigger | Output |
| :--- | :--- | :--- |
| Scan | quick pass | <=8 bullets: weakest claim, weakest citation, top stat risk |
| Evaluate | logic only | Reasoning audit |
| Fact-check | @fact-check | Claims + references + stats tables + Fix/Soften/Check |
| Full | draft audit | Fact-check section, then reasoning |

### Fact-check schema

Claims, References (tier + issue), Statistics (if any), then Fix / Soften / Check. English labels only.

### Evaluate schema

Main claim; Structure; Hidden premises; Evidence + reference gaps; Stats issues (if any); Fallacy/bias (named only if in `evaluate.md`); Confidence + what would change it.

## 7. Style

Note:, Warning:, Assumption:, Confidence:. No em dashes. English throughout.
