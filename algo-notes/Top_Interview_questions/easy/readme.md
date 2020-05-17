
## 2. Add Two Numbers

You are given two non-empty linked lists representing two non-negative integers. The digits are stored in reverse order and each of their nodes contain a single digit. Add the two numbers and return it as a linked list.

You may assume the two numbers do not contain any leading zero, except the number 0 itself.

```
Example:

Input: (2 -> 4 -> 3) + (5 -> 6 -> 4)
Output: 7 -> 0 -> 8
Explanation: 342 + 465 = 807.
```

### Solution

```cpp
/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode() : val(0), next(nullptr) {}
 *     ListNode(int x) : val(x), next(nullptr) {}
 *     ListNode(int x, ListNode *next) : val(x), next(next) {}
 * };
 */
class Solution {
public:
    ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
        int c = 0;
        ListNode *iter1 = l1;
        ListNode *iter2 = l2;
        ListNode *sen = new ListNode(0);
        ListNode *iter3 = sen;
        while(iter1!=NULL || iter2!=NULL){
            int aa = 0;
            int bb = 0;
            if(iter1!=NULL) aa = iter1->val;
            if(iter2!=NULL) bb = iter2->val;
            int k = aa+bb+c;
            c = k/10;
            ListNode *node = new ListNode(k%10);
            
            iter3->next = node;
            iter3 = iter3->next;
            
            if(iter1!=NULL) iter1 = iter1->next;
            if(iter2!=NULL) iter2 = iter2->next;
        }
        
        if(c>0){
            ListNode *node = new ListNode(c);
            iter3->next = node;
            iter3 = iter3->next;
        }
        return sen->next;
        
    }
};

```

```cpp
/**
 * Definition for singly-linked list.
 * struct ListNode {
 *     int val;
 *     ListNode *next;
 *     ListNode(int x) : val(x), next(NULL) {}
 * };
 */
class Solution {
public:
    ListNode* addTwoNumbers(ListNode* l1, ListNode* l2) {
        ListNode * sen = new ListNode(-1);
        ListNode *iter =  sen;
        int carry = 0;
        while(l1!=NULL && l2!=NULL){
            
            int sm = carry + l1->val + l2->val;
            carry = sm /10;
            sm = sm%10;
            iter->next = new ListNode(sm);
            iter = iter->next;
            l1 = l1->next;
            l2 = l2->next;
        }
        
        while(l1!=NULL){
            int sm = carry + l1->val;
            carry = sm /10;
            sm = sm%10;
            iter->next = new ListNode(sm);
            iter = iter->next;
            l1 = l1->next;
        }
        while(l2!=NULL){
            int sm = carry + l2->val;
            carry = sm /10;
            sm = sm%10;
            iter->next = new ListNode(sm);
            iter = iter->next;
            l2 = l2->next;
        }
        
        if(carry){
            iter->next = new ListNode(1);
        }
        return sen->next;
        
    }
};
```