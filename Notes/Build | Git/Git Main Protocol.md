# Git Protocol for Updating cursor-main

This document outlines the recommended approaches for updating the cursor-main branch while avoiding painful conflict resolution.

## The "Right Way" to Update cursor-main (Cherry-pick Method)

This approach is ideal for targeted changes and avoids rebasing entirely.

1. **Create a new clean branch from cursor-main**:
   ```bash
   git fetch origin
   git checkout -b temp-update-cursor-main origin/cursor-main
   ```

2. **Cherry-pick your new changes from cursor-development**:
   ```bash
   # For a single commit
   git cherry-pick cursor-development
   
   # For multiple specific commits
   git cherry-pick commit1 commit2 commit3
   
   # For a range of commits (all commits from A to B, inclusive)
   git cherry-pick A^..B
   ```

3. **Push this branch and create a PR**:
   ```bash
   git push origin temp-update-cursor-main
   ```

4. **Create a PR on GitHub** from temp-update-cursor-main to cursor-main

## The Batch Resolution Rebase Approach

Use this approach when you need to rebase a branch with many commits onto cursor-main.

1. **Start with your development branch**:
   ```bash
   git checkout cursor-development
   
   # Create a temporary branch for the rebase
   git checkout -b temp-for-rebase
   ```

2. **Begin the rebase**:
   ```bash
   git fetch origin
   git rebase origin/cursor-main
   ```

3. **When conflicts arise, use batch resolution**:
   ```bash
   # Accept ALL changes from the incoming branch for directories
   git checkout --theirs -- shared/
   git add shared/
   
   git checkout --theirs -- standalone/
   git add standalone/
   
   git checkout --theirs -- Notes/
   git add Notes/
   
   # For specific files
   git checkout --theirs -- CMakeLists.txt
   git add CMakeLists.txt
   
   git checkout --theirs -- build.sh
   git add build.sh
   ```

4. **Continue the rebase**:
   ```bash
   git rebase --continue
   ```

5. **Repeat steps 3-4 for each conflicting commit** until the rebase is complete

6. **Push the rebased branch**:
   ```bash
   git push -f origin temp-for-rebase
   ```

## Tips for Handling Commit Message Issues

If you encounter issues with the commit message editor during rebase:

1. **Set a simpler editor temporarily**:
   ```bash
   export EDITOR=cat
   ```
   Or:
   ```bash
   GIT_EDITOR=true git rebase --continue
   ```

2. **For files not covered by batch commands, add them specifically**:
   ```bash
   git checkout --theirs -- path/to/specific/file
   git add path/to/specific/file
   ```

3. **If the rebase gets too complex, you can always abort**:
   ```bash
   git rebase --abort
   ```
   And try the cherry-pick approach instead.

## Prevention Strategy

To avoid needing these techniques in the future:

1. **Regularly pull from cursor-main into your development branch**:
   ```bash
   git checkout cursor-development
   git pull origin cursor-main
   # Resolve any conflicts (usually much fewer)
   git push
   ```

2. **Use feature branches for specific changes**:
   ```bash
   git checkout -b feature/my-new-feature cursor-development
   # Make changes, then merge back to cursor-development when done
   ```

3. **Keep commits smaller and focused** - This makes conflict resolution easier when it does happen. 