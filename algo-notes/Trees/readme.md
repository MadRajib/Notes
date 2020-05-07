# Trees

---

## DFS

### Pre-order

```python
// Recursive
def preorder(node):
    if (node == null):
        return
    visit(node)
    preorder(node.left)
    preorder(node.right)

// Iterative
def iterativePreorder(node):
  if (node == null):
        return
  s ← empty stack
  s.push(node)
  while (not s.isEmpty()):
    node ← s.pop()
    visit(node)
    //right child is pushed first so that left is processed first
    if (node.right is not None):
      s.push(node.right)
    if (node.left is not None):
      s.push(node.left)

```



### In-order

```cpp

// Recursive
inorder(node)
    if (node == null)
        return
    inorder(node.left)
    visit(node)
    inorder(node.right)

// Iterative
vector<int> inorderTraversal(TreeNode* root) {
        vector<int> ans;
        stack <TreeNode *> s;
        
        while(root!=NULL || !s.empty()){
            while(root!=NULL){
                s.push(root);
                root = root->left;
            }
            TreeNode *n = s.top();
            s.pop();
            ans.push_back(n->val);
            root = n->right;
            
        }
        
        return ans;
}

```

### Post-order

```python
// Recursive
def postorder(node):
    if (node == null):
        return
    postorder(node.left)
    postorder(node.right)
    visit(node)

// Iterative
def iterativePostorder(node):
  s ← empty stack
  lastNodeVisited ← null
  while (not s.isEmpty() or node ≠ null):
    if (node ≠ null)
      s.push(node)
      node ← node.left
    else
      peekNode ← s.peek()
      // if right child exists and traversing node
      // from left child, then move right
      if (peekNode.right ≠ null and lastNodeVisited ≠ peekNode.right)
        node ← peekNode.right
      else
        visit(peekNode)
        lastNodeVisited ← s.pop()
        node ← null

```


## BFS

```python
def levelorder(root):
    q ← empty queue
    q.enqueue(root)
    while not q.isEmpty() do
        node ← q.dequeue()
        visit(node)
        if node.left ≠ null then
            q.enqueue(node.left)
        if node.right ≠ null then
            q.enqueue(node.right)

```