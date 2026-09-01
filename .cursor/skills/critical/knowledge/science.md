# Science: references, evidence, and statistics

Audit empirical claims, citations, and quantitative reasoning in pasted prose.

**Pair with:** `evaluate.md` for argument logic; read project design notes or references when claims must be checked against primary sources.

## Reference tiers (label each citation)

| Tier | Type | Weight for factual claims |
| :--- | :--- | :--- |
| A | Peer-reviewed primary research (journal article, systematic review, meta-analysis) | Strong if methods fit the claim |
| B | Peer-reviewed narrative review or authoritative handbook chapter | Good for overview; weak for novel specifics |
| C | Preprint, thesis, conference abstract | Preliminary; not yet peer-reviewed |
| D | Grey literature (report, guideline, institution page) | Check author, date, evidence cited |
| E | Popular press, blog, encyclopedia, textbook summary | Starting point only; trace to A or B |
| F | Unknown, broken, or misattributed | Unsupported until fixed |

**Peer review** means independent expert review before publication. It raises quality bar; it does not guarantee truth. Retractions, conflicts of interest, and weak methods still matter.

## Citation audit (per reference)

1. **Match:** Does the source say what the text claims? Quote vs paraphrase vs overreach?
2. **Scope:** Population, species, dose, geography, year: same as in the draft?
3. **Strength:** Tier A/B with fitting design, or weaker?
4. **Currency:** Superseded by later work?
5. **Trace:** Tier E claims need an A/B source behind them.

Flag: **misquote**, **overgeneralization**, **wrong study**, **tier too weak**, **missing citation**, **[needs verification]**.

## Empirical claim labels

| Label | Meaning |
| :--- | :--- |
| **Established** | Consistent A-tier evidence or broad scientific consensus |
| **Supported** | At least one credible A/B source; limited but adequate for scope |
| **Preliminary** | C-tier, single small study, or conflicting results |
| **Unsupported** | No adequate source, or source does not support the claim |
| **Overstated** | Direction may be right but magnitude, causality, or certainty is inflated |
| **Wrong** | Contradicted by cited source or stronger evidence |

## Scientific reasoning checklist

1. **Question:** Is the research question clear and testable?
2. **Hypothesis:** Stated before results? Distinguish prediction from speculation.
3. **Design:** RCT, cohort, case-control, cross-sectional, case series, in vitro, model: does design support the causal or mechanistic claim made?
4. **Controls / comparison:** Appropriate baseline or control group?
5. **Confounders:** Alternative explanations ruled out or acknowledged?
6. **Measurement:** Valid, reliable instruments; blinded where relevant?
7. **Replication:** Single lab vs replicated; independent confirmation?
8. **Inference:** Conclusion within what the study actually measured?

**Design vs claim:** observational data cannot alone prove causation; mechanistic plausibility plus intervention evidence is stronger.

## Statistical methods audit

Ask whether the analysis matches the data and the claim.

| Check | Question |
| :--- | :--- |
| Sample size | Large enough for the effect claimed? Underpowered? |
| Representativeness | Sample matches target population? Selection bias? |
| Effect vs significance | p-value cited without effect size, CI, or clinical/biological importance? |
| Multiple comparisons | Many tests without correction inflating false positives? |
| Wrong test | Means vs medians, paired vs unpaired, categorical vs continuous? |
| Extrapolation | Animal, in vitro, or narrow cohort generalized to humans or field? |
| Causal language | "Caused", "proved", "shows" from correlation or single study? |
| Subgroup fishing | Post-hoc subgroups presented as prespecified? |
| Missing data | Dropouts, exclusions, or missingness biasing results? |
| Absolute vs relative risk | Relative risk amplified without base rates? |

Reuse bias patterns from `evaluate.md` (base rate, hasty generalization, regression to mean, false cause) when they fit.

## Probabilistic reporting

- State **confidence** (high / medium / low) and what would change it.
- Separate **decision quality** from **outcome**: a reasonable conclusion can fail by chance.
- Update when new evidence appears; do not treat one study as final proof.

## Draft + reference output schema

**Claims:** `- <claim> -> established | supported | preliminary | unsupported | overstated | wrong | [needs verification]`

**References:** `- <citation as written> -> tier A-F | issue (misquote / overreach / weak tier / ...)`

**Statistics:** `- <stat claim> -> OK | overstated | unsupported | method flaw: <short label>`

Then group **Fix** / **Soften** / **Check** as in `evaluate.md`.

Do not invent DOIs, page numbers, or study results. If sources are not pasted or readable, mark **[needs verification]** and name what to open.
