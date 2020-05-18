
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

## 8. String to Integer (atoi)

### Solution

```cpp
class Solution {
public:
    int myAtoi(string str) {
        long res = 0;
        int i = 0;
        int sign = 1;
        while(i < str.length() && str[i] == ' ') i++;
        
        if(str[i] == '-' || str[i] == '+'){
            if(str[i] == '-') sign =-1;
            i++;
        }
        
        while(isdigit(str[i])){
            short v = str[i] - '0'; 
            res = res*10 + v;
            if(res > INT_MAX){
                
                return (sign>0)?2147483647:-2147483648;
            }
            i++;
        }
        
        return res*sign;
    }
};
```


## 29. Divide Two Integers

Given two integers dividend and divisor, divide two integers without using multiplication, division and mod operator.

Return the quotient after dividing dividend by divisor.

The integer division should truncate toward zero, which means losing its fractional part. For example, truncate(8.345) = 8 and truncate(-2.7335) = -2.
```
Example 1:

Input: dividend = 10, divisor = 3
Output: 3
Explanation: 10/3 = truncate(3.33333..) = 3.
Example 2:

Input: dividend = 7, divisor = -3
Output: -2
Explanation: 7/-3 = truncate(-2.33333..) = -2.
Note:

Both dividend and divisor will be 32-bit signed integers.
The divisor will never be 0.
Assume we are dealing with an environment which could only store integers within the 32-bit signed integer range: [−231,  231 − 1]. For the purpose of this problem, assume that your function returns 231 − 1 when the division result overflows.
```

### Solution:

```cpp
class Solution {
public:
    int divide(int dividend, int divisor) {
    
        if(dividend == 0) return 0;

        if(divisor == 1) return dividend;

        if(divisor == -1) return (dividend == INT_MIN)? INT_MAX:-1*dividend;
        if(divisor == INT_MIN ) return (dividend==INT_MIN)?1:0;

        int res=0;
    
        
        if(dividend==INT_MIN){
            res++;
            dividend += abs(divisor);
        }

        int a = abs(dividend);
        int b = abs(divisor);
        
        while(a>=b){
            res += helper(a,b);
        }

        return  ((dividend>0) == (divisor>0)) ? res : -res;
    }
    
    int helper(int& n, int x) {
        int res = 1;
        while( n-x>=x ){  
            x = x<<1;         
            res += res;       
        }  
        n = n-x;            
       return res;
    }
  
};
```


## 50. Pow(x, n)

Implement pow(x, n), which calculates x raised to the power n (xn).

### Solution

```cpp
class Solution {
public:
    double myPow(double x, int p) {
        long n = p;
        
        double res =1;
        
        // If n is negative change x and make power to psoitve
        if(n < 0){
            x = 1/x;
            n *=-1;
        }
        
        // convert power to binary just in fast exponent process 
        while(n){
            
            if(n&1) res *=x;
            
            n= n>>1;
            x*=x;
        }
        
        
        return res;
    }
    
};

```