# Workflows

## Manage Fork

This repo is a fork of [joeycastillo/second-movement](https://github.com/joeycastillo/second-movement). Remotes and branches are managed as follows:

- `upstream/main` - joeycastillo's code (read-only)
- `origin/main`   - this fork's mirror (kept in sync with upstream)
- `origin/custom` - this fork's changes

### Setup on New Machine

```bash
git clone git@github.com:mark-kimmerman/second-movement.git
cd second-movement
git remote add upstream https://github.com/joeycastillo/second-movement.git
git fetch upstream
git checkout custom    # tracks origin/custom automatically
```

### Sync `main` with upstream

```bash
git fetch upstream
git checkout main
git merge --ff-only upstream/main # --ff-only aborts if main has diverged
                                  # (keeps a clean mirror)
git push origin main
```

### Pull Updates into Working Branch

```bash
git checkout custom
git fetch upstream
git rebase upstream/main
git push --force-with-lease origin custom
```

