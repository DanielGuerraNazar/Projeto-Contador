#!/bin/bash
# commit_and_push.sh - Automates versioned commit, tag and push according to project rules
# Usage: ./commit_and_push.sh "Brief summary of changes" [level]
#        level can be: major, minor, patch (default: patch)

set -e

REPO_DIR="$(git rev-parse --show-toplevel)"
cd "$REPO_DIR"

# 1. Pull latest changes (rebase) to avoid divergence
git pull --rebase origin main || true

# 2. Determine next version tag (Semantic Versioning)
latest_tag=$(git tag --sort=-v:refname | head -n1 || echo "v0.0.0")
# Strip leading 'v'
ver=${latest_tag#v}
IFS='.' read -r major minor patch <<< "$ver"
# Default to 0.0.0 if parsing fails
if [[ -z "$major" || -z "$minor" || -z "$patch" ]]; then
  major=0; minor=0; patch=0
fi

# Determine bump level (default patch)
level="${2:-patch}"
case "$level" in
  major)
    next_major=$((major + 1))
    next_minor=0
    next_patch=0
    ;;
  minor)
    next_major=$major
    next_minor=$((minor + 1))
    next_patch=0
    ;;
  patch|*)
    next_major=$major
    next_minor=$minor
    next_patch=$((patch + 1))
    ;;
esac
new_tag="v${next_major}.${next_minor}.${next_patch}"

# 3. Get current date
today=$(date +%d-%m-%Y)

# 4. Build commit message
if [ -z "$1" ]; then
  summary="Atualizações"
else
  summary="$1"
fi
commit_msg="[Versão ${next_major}.${next_minor}.${next_patch}] - $today ($summary)"

# 5. Stage all changes and commit
git add .
# If there is nothing to commit, exit gracefully
if git diff-index --quiet HEAD --; then
  echo "No changes to commit."
else
  git commit -m "$commit_msg"
fi

# 6. Tag the new version
git tag "$new_tag"

# 7. Generate / update CHANGELOG.md
# Get previous tag (if any) to list commits since then
previous_tag=$(git tag --sort=-v:refname | grep -v "$new_tag" | head -n1)
if [ -z "$previous_tag" ]; then
  log_range="HEAD"
else
  log_range="${previous_tag}..HEAD"
fi
# Create changelog entry
changelog_entry="## ${new_tag} - $(date +%Y-%m-%d)\n$(git log $log_range --pretty=format:"- %s")\n"
# Prepend to CHANGELOG.md (create if missing)
if [ -f CHANGELOG.md ]; then
  tmp=$(mktemp)
  echo -e "$changelog_entry\n$(cat CHANGELOG.md)" > "$tmp"
  mv "$tmp" CHANGELOG.md
else
  echo -e "$changelog_entry" > CHANGELOG.md
fi

# 8. Push commits and tags
git push origin main --tags

echo "✅ Pushed version $new_tag with message: $commit_msg"
