## Notes referenced from Beej's guide to git [text](https://beej.us/guide/bggit/)

1. To configure your name and email global
```bash
$ git config set --global user.name "Your name"
$ git config set --global user.email "your-email@example.com"
```
2. To rename branch name from __master__ to other 
```bash
$ git config set --global init.defaultBranch main
```
3. To view the diff for things in staged
```bash
$ git diff --staged
```
4. To unstage a file added accidently
```bash
$ git restore --staged hello.py
```
5. To commit a change in a file without staging it. Works for both tracked and untracked files.
```bash
$ git commit -m "foo bar" foo.txt
```
6. 
