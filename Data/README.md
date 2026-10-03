# Canonical catalogs

Runtime catalogs live in `Game/Daedalus/Content/Data/` so the same source is
staged into packaged games. Do not maintain a second editable copy here.
The current foundation catalog is entirely original synthetic test content,
not astronomy data, licensed franchise models, or approved game balancing.

Each definition has a permanent namespaced ID. Renaming display text is safe;
changing IDs requires an explicit save migration. Versioned JSON is validated
transactionally before any live state changes. Large models are referenced and
loaded by presentation separately; catalog loading must not load every model.
