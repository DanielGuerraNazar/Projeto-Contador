#!/bin/bash
# commit_and_push.sh - Automates versioned commit, tag and push according to project rules
# Usage: ./commit_and_push.sh "Brief summary of changes"

set -e

REPO_DIR="$(git rev-parse --show-toplevel)"
cd "$REPO_DIR"

# 1. Pull latest changes (rebase) to avoid divergence
git pull --rebase origin main || true

# 2. Determine next version tag (integer major increment)
latest_tag=$(git tag --sort=-v:refname | head -n1 || echo "v0.0")
if [[ -z "$latest_tag" ]]; then
  next_major=1
else
  # Strip leading 'v'
  ver=${latest_tag#v}
  IFS='.' read -r major minor <<< "$ver"
  next_major=$((major + 1))
fi
new_tag="v${next_major}.0"

# 3. Get current date
today=$(date +%d-%m-%Y)

# 4. Build commit message
if [ -z "$1" ]; then
  summary="Atualizações"
else
  summary="$1"
fi
commit_msg="[Versão ${next_major}.0] - $today ($summary)"

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

# 7. Push commits and tags
git push origin main --tags

echo "✅ Pushed version $new_tag with message: $commit_msg"
