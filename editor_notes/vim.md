## search for a text and open the file 
```bash
$ vim .
// in vim
: grep -inr <word to search> .
:copen // :cn for next :cp for previous
//Cntl + w + arrow keys to jump up and down
```

## git blame the line
:!git blame -L lineno,lineno %