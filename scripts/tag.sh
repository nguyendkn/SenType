#!/usr/bin/env bash
# Cut a release tag: v<VERSION>.<YYYYMMDD><INDEX>  (VERSION = "MAJOR.MINOR" from the top-level VERSION file)
# INDEX is 0 for the first tag of the day, then 1, 2, ... Pushing the tag triggers .github/workflows/release.yml.
#
# Usage:  scripts/tag.sh [--dry-run]        REMOTE=origin scripts/tag.sh   (skips the remote picker)
set -euo pipefail

DRY_RUN=0
case "${1:-}" in
  "") ;;
  --dry-run|-n) DRY_RUN=1 ;;
  -h|--help) sed -n '2,6p' "$0"; exit 0 ;;
  *) echo "❌ Unknown argument: $1 (use --dry-run)"; exit 1 ;;
esac

cd "$(git rev-parse --show-toplevel)"

# --- version (single source of truth: ./VERSION) ---
VERSION="$(tr -d '[:space:]' < VERSION 2>/dev/null || true)"
if [[ ! "$VERSION" =~ ^[0-9]+\.[0-9]+$ ]]; then
  echo "❌ VERSION file must contain MAJOR.MINOR (e.g. 0.1), got: '${VERSION}'"; exit 1
fi
echo "📦 Version: $VERSION"

# --- clean tree (tracked files; untracked files cannot change what gets tagged) ---
git update-index -q --refresh || true  # drop stat-only noise (touched, unchanged files)
if ! git diff-index --quiet HEAD -- 2>/dev/null; then
  echo "❌ Working tree has uncommitted changes to tracked files. Commit or stash them first."; exit 1
fi

# --- remote ---
REMOTES=()
while IFS= read -r r; do [ -n "$r" ] && REMOTES+=("$r"); done < <(git remote) || true
if [ "${#REMOTES[@]}" -eq 0 ]; then echo "❌ This repository has no git remote."; exit 1; fi
REMOTE="${REMOTE:-}"
if [ -n "$REMOTE" ]; then
  if ! printf '%s\n' "${REMOTES[@]}" | grep -qxF -- "$REMOTE"; then
    echo "❌ Unknown remote: $REMOTE (have: ${REMOTES[*]})"; exit 1
  fi
elif [ "${#REMOTES[@]}" -eq 1 ]; then
  REMOTE="${REMOTES[0]}"; echo "✅ Only remote: $REMOTE"
else
  if [ ! -t 0 ]; then echo "❌ Several remotes (${REMOTES[*]}) and no TTY: set REMOTE=<name>."; exit 1; fi
  echo "Select remote:"
  for i in "${!REMOTES[@]}"; do echo "$((i + 1))) ${REMOTES[$i]}"; done
  printf "Enter choice [1-%s]: " "${#REMOTES[@]}"; read -r choice
  if [[ ! "$choice" =~ ^[0-9]+$ ]] || [ "$choice" -lt 1 ] || [ "$choice" -gt "${#REMOTES[@]}" ]; then
    echo "❌ Invalid option."; exit 1
  fi
  REMOTE="${REMOTES[$((choice - 1))]}"; echo "✅ Selected remote: $REMOTE"
fi

# --- sync: stale tag lists would produce a duplicate INDEX, so a failed fetch is fatal ---
echo "🔍 Fetching branches and tags from $REMOTE..."
if ! git fetch --tags --quiet "$REMOTE"; then echo "❌ git fetch $REMOTE failed."; exit 1; fi

# --- HEAD must already be on the remote (the workflow builds the pushed commit) ---
if [ -z "$(git for-each-ref --count=1 --contains HEAD "refs/remotes/$REMOTE")" ]; then
  echo "❌ HEAD ($(git rev-parse --short HEAD)) is not on any $REMOTE branch. Push it first."; exit 1
fi

# --- refuse to tag a commit that already carries a release tag ---
EXISTING="$(git tag --points-at HEAD -l 'v[0-9]*.[0-9]*.[0-9]*' | head -1)"
if [ -n "$EXISTING" ]; then echo "❌ HEAD is already tagged $EXISTING."; exit 1; fi

# --- next index: plain string ops, no regex built from tag text ---
DATE="$(date +%Y%m%d)"
BASE_TAG="v$VERSION.$DATE"
LAST=-1
while IFS= read -r t; do
  [ -n "$t" ] || continue
  suffix="${t#"$BASE_TAG"}"
  if [ -z "$suffix" ]; then n=0                                # bare BASE_TAG counts as index 0
  elif [[ "$suffix" =~ ^[0-9]+$ ]]; then n=$((10#$suffix))
  else continue; fi
  if [ "$n" -gt "$LAST" ]; then LAST=$n; fi
done < <(git tag -l "$BASE_TAG*")
INDEX=$((LAST + 1))
TAG="$BASE_TAG$INDEX"

if [ "$DRY_RUN" -eq 1 ]; then
  echo "🧪 Dry run: would create and push $TAG → $REMOTE (commit $(git rev-parse --short HEAD))"; exit 0
fi

echo "🏷️  Creating tag: $TAG"
git tag -a "$TAG" -m "SenType $TAG"
if ! git push "$REMOTE" "refs/tags/$TAG"; then
  git tag -d "$TAG" >/dev/null; echo "❌ Push failed; local tag $TAG removed."; exit 1
fi
echo "✅ Successfully tagged and pushed: $TAG → $REMOTE"
