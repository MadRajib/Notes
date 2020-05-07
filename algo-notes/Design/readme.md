# Design

---

## 146. LRU Cache

Design and implement a data structure for Least Recently Used (LRU) cache. It should support the following operations: get and put.

get(key) - Get the value (will always be positive) of the key if the key exists in the cache, otherwise return -1.
put(key, value) - Set or insert the value if the key is not already present. When the cache reached its capacity, it should invalidate the least recently used item before inserting a new item.

The cache is initialized with a positive capacity.

Follow up:
Could you do both operations in O(1) time complexity?
```
Example:

LRUCache cache = new LRUCache( 2 /* capacity */ );

cache.put(1, 1);
cache.put(2, 2);
cache.get(1);       // returns 1
cache.put(3, 3);    // evicts key 2
cache.get(2);       // returns -1 (not found)
cache.put(4, 4);    // evicts key 1
cache.get(1);       // returns -1 (not found)
cache.get(3);       // returns 3
cache.get(4);       // returns 4

```

## Approach

We will use two data structures :

1. Hash Table
    Hash table to store the nodes.

1. And Doubly Linked list.
    It is used for ordering the nodes.

Most recently used nodes will be in the front of the list and the least at the end.  

Whenever we add a node to the list we will add it to the front.

Whenever we want to remove a node (due to over capacity) we will remove the last node.

A node will have 2 values (key and value) and 2 links( left and right).

we are saving the key so that we can remove a node from the hash map using the node key.

when a **GET** request for a key is called,  
if its in the hash map:

1. we get its node from the hash table.

1. And adds it to the front.

> To add to front if the key is present in the map,
we first pull it from the list and then add it in the front of the list.  

If it is not in the table for a **GET** we return -1.

For a **PUT** request:

1. we check if the key already exits or not,  
    if exits we updates it value with the new value  
    and add it back to the front.

1. if the value not exits we first check if the capacity is full.
    Now we need to make room for the new node.  
    If it is we remove the last node from the list and also remove it key from the table.

    Then we create a new node assign it the value and key, and add it to both list and hashmap.

1. else we just create a new node assign it the value and key, and add it to both list and hashmap.

## Solution

```cpp
class LRUCache {
    private:
    struct Node{
        int key;
        int val;
        Node *left = NULL;
        Node *right = NULL;

        Node() = default;
        Node(int v,int k):val(v),key(k){}
    };
    
    Node *head = new Node();
    Node *tail = new Node();
    
    unordered_map<int, Node*> map;
    int cache_capacity;
    
    
    // remove the node from the list
    void remove(Node *n){
        n->left->right = n->right;
        n->right->left = n->left;
        
        n->left = NULL;
        n->right = NULL;
    }
    
    // add to the front of the list
    void add(Node *n){
        n->left = head;
        n->right = head->right;
        head->right = n;
        n->right->left = n;
        
    }
    
    public:
    
    LRUCache(int capacity) {
        cache_capacity = capacity;
        
        head->right = tail;
        tail->left = head;
    }
    
    int get(int key) {
        auto node = map.find(key);
        if(node!=map.end()){
            
            auto n = node->second;
            
            // remove it from the middle and put it
            // in the front of the lis

            remove(n);
            add(n);

            return n->val;
        }else{
            return -1;
        }
    }
    
    void put(int key, int value) {
        auto node = map.find(key);
        
        if(node!=map.end()){
            // if the key is present update its node's value and add it to the front.

            auto n = node->second; 
            n->val = value;

            // aadding to the front
            remove(n);
            add(n);
            
        }else{
             
             
            if(map.size() == cache_capacity){
                
                // If capacity is full remove the last node
                auto n = tail->left;
                
                // remove the key and its nodefrom the hashmap and the list
                map.erase(n->key);
                remove(n);
                delete(n);
            }
            
            // Add a key and new node to the hash map and list
            Node *new_node = new Node(value,key);
            
            map.insert({key,new_node});
            
            // add to the front
            add(new_node);
            
        } 

    }
};

```
