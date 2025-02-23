user## Notes referenced from Beej's guide to git [text](https://beej.us/guide/bggit/)

__1.__ To configure your name and email global
```bash
$ git config --global user.name "Your name"
$ git config --global user.email "your-email@example.com"
```
__2.__ To rename branch name from __master__ to other 
```bash
//NOTE: This setting only applies to new repositories initialized with git init.
$ git config --global init.defaultBranch main
//NOTE: Existing repositories will not be affected. If you need to rename an existing branch from master to main
$ git branch -m master main
```

__3.__ To view the diff for things in staged
```bash
$ git diff --staged
```
__4.__ To unstage a file added accidently
```bash
$ git restore --staged hello.py
```
__5.__ To commit a change in a file without staging it. Works for both tracked and untracked files.
```bash
$ git commit -m "foo bar" foo.txt
```
__6.__ To switch to a specific commit
```bash
$ git switch --detach 5a02fe

HEAD is now at 5a02fed
$ git log
commit 5a02fede3007edf55d18e2f9ee3e57979535e8f2 (HEAD)
Author: User <user@example.com>
Date:   Thu Feb 1 09:24:52 2024 -0800

    Added
```
This will detach the HEAD from main and point
to the specific commit. HEAD is now in Detach state

Head intial points to main branch
```bash
commit 5e8cb52cb813a371a11f75050ac2d7b9e15e4751 (HEAD -> main)
Author: User Name <user@example.com>
Date:   Thu Feb 1 12:36:13 2024 -0800

    More output

commit 5a02fede3007edf55d18e2f9ee3e57979535e8f2
Author: User Name <user@example.com>
Date:   Thu Feb 1 09:24:52 2024 -0800

    Added
```
__7.__ To reattach the head
```bash
// jump to previous place
$ git switch -
// jump to main explicitly
$ git switch main
  Previous HEAD position was 5a02fed Added
  Switched to branch 'main'
```
__8.__ To deatch head from main and point to commit
```bash
$ git switch --detach HEAD
```
__9.__ To detach HEAD to one commit before
```bash
$ git switch --detach HEAD^
// can use @ in place of HEAD
$ git switch --detach @^     # Same thing
```
To go to 3rd previous commit
```bash
$ git switch --detach HEAD^^^
```
Better way
```bash
$ git switch --detach HEAD~3
```
__10.__ git merge commit vs git rebase

If remote is ahead of local repo, if git pull is performed
by default a new merge commit is created.
```bash
A -- B -- C (remote)
A -- B -- D (local)

$ git pull
-> feteches the latest changes
-> merges the remote changes into your local branch
A -- B -- C (remote)
A -- B -- D  -- M(merge commit) (local)
```
if run pull with rebase then:
```bash
$ git pull
A -- B -- C (remote)
A -- B -- C -- D (local)
```

A merge commit is a special commit that gets created
when merging two branches. It has two parents commit.
eg :
```bash
A -- B -- C   (main)

A -- B -- C  (main)
       \
        D -- E  (feature)

$ git checkout main
$ git merge feature

       D -- E  (feature)
      /        
A -- B -- C -- M  (main)

M is the merge commit that joins the changes from both the commit.
```
* Preserves History – It keeps the history of both branches as they were before merging.

It we perform git rebase before we pull from remote
```bash
A -- B -- C (main)
          \
           X -- Y (feature)

$ git checkout feature
$ git rebase main

A -- B -- C -- X' -- Y' (main)
```
* Origianl history of feature is lost

__11.__ To perform rebase during pull
```bash
$ git config --global pull.rebase false
```

Conclusion
* git pull with merge (pull.rebase=false) creates a merge commit when branches diverge.
* git pull --rebase rebases your commits on top of the remote changes, making the history linear.


__12.__ Recalling that the main branch is just a label for a specific commit, how does the main branch know to “follow” our HEAD from commit to commit?

It does it like this: the branch that HEAD points to follows the current commit. That is, when you make a commit, the branch HEAD points to moves along to that next commit.

Contrast that to detached head state. If we were there, a new commit would get us to a new state, leaving main alone.