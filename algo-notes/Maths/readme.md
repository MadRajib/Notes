## 7. Reverse Integer

Given a 32-bit signed integer, reverse digits of an integer.

```
Example 1:

Input: 123
Output: 321
Example 2:

Input: -123
Output: -321
Example 3:

Input: 120
Output: 21

Note:
Assume we are dealing with an environment which could only store integers within the 32-bit signed integer range: [−231,  231 − 1]. For the purpose of this problem, assume that your function returns 0 when the reversed integer overflows.
```

## Solution

```cpp
class Solution {
public:
    int reverse(int x) {
       long res = 0;
       while(x){
        
            res = 10*res +x%10;
            x /= 10;
           if((res > INT_MAX) || (res < INT_MIN)) return 0;
           
       }
        
        return (int)res;
    }
};

```

## 9. Palindrome Number

Determine whether an integer is a palindrome. An integer is a palindrome when it reads the same backward as forward.

```

Example 1:

Input: 121
Output: true
Example 2:

Input: -121
Output: false
Explanation: From left to right, it reads -121. From right to left, it becomes 121-. Therefore it is not a palindrome.
Example 3:

Input: 10
Output: false
Explanation: Reads 01 from right to left. Therefore it is not a palindrome.
Follow up:

Coud you solve it without converting the integer to a string?

```

### Solution

```cpp
class Solution {
public:
    bool isPalindrome(int x) {
        
        if(x < 0) return 0;
        
        int strt = x;
        long res = 0;
        while(x){
            res = res*10 + x%10;
            x/=10;
            if(res > INT_MAX || res < INT_MIN) return 0;
        }
        return strt == res;
    }
};
```

## 13. Roman to Integer

### Solution

```cpp
unordered_map<char,int> roman_order = {{'I',1},
                                    {'V',5},
                                    {'X',10},
                                    {'L',50},
                                    {'C',100},
                                    {'D',500},
                                    {'M',1000}};

class Solution {
public:
    int romanToInt(string s) {
        int prev = 0;
        int res = 0;
        for(auto iter = s.rbegin(); iter != s.rend() ;iter++){
                    
            auto val = roman_order.find(*iter)->second;
            if(val >= prev){
                res+=val;
            }else{
                res-=val;
            }
            
            prev = val;
        }
        return res;
    }
};

```

## 67. Add Binary

Given two binary strings, return their sum (also a binary string).

The input strings are both non-empty and contains only characters 1 or 0.

```

Example 1:

Input: a = "11", b = "1"
Output: "100"
Example 2:

Input: a = "1010", b = "1011"
Output: "10101"

```

### Solution

```cpp
class Solution {
public:
    string addBinary(string a, string b) {
        
        string res;
        auto iter1 = a.rbegin();
        auto iter2 = b.rbegin();
        int c = 0;
        while(iter1 < a.rend() || iter2 < b.rend()){
            int b1 = 0 ,b2 = 0;
            if(iter1 < a.rend()) b1 = *iter1 - '0';
            if(iter2 < b.rend()) b2 = *iter2 - '0';
            
            int v = b1+b2 + c;
            c = v/2;
            v = v%2;
            res.push_back((v)?'1':'0');
            iter1++;
            iter2++;
        }
        
        if(c>0) res.push_back('1');
        reverse(res.begin(),res.end());
        return res;
        
    }
};
```