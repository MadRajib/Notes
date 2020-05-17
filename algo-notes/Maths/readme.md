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

## 69. Sqrt(x)

### Solution

```cpp
class Solution {
public:
    int mySqrt(int x) {
        int l = 0,h = x;
        while(l<=h){
            long long m = l + (h-l)/2;
            long long srt = m*m;
            if(srt == x) return m;
            if(srt < x) l = m+1;
            else h = m-1;
        }
        
        return l-1;
    }
};

```

## 168. Excel Sheet Column Title

### Solution

```cpp
class Solution {
public:
    string convertToTitle(int n) {
        string res;
        while(n){
            int x = (n-1)%26;
            res.push_back(char(65+x)) ;
            n = (n-1)/26;
        }
        reverse(res.begin(),res.end());
        return res;
    }
};

```

## 171. Excel Sheet Column Number

### Solution

```cpp
class Solution {
public:
    int titleToNumber(string s) {
        
        auto iter = s.rbegin();
        int res = 0;
        long r = 1;
        while(iter != s.rend()){
            int v = *iter - 'A' + 1;
            
            res += r*v;
            r*=26;
            
            iter++;
            
        }
        
        return res;
    }
};

```

## 172. Factorial Trailing Zeroes

Given an integer n, return the number of trailing zeroes in n!.

### Solution

```cpp
class Solution {
public:
    int trailingZeroes(int n) {
        int res = 0;
        while(n>4){
            res += n/5;
            n /= 5;
        }
        
        return res;
    }
};


```


## 202. Happy Number

Write an algorithm to determine if a number n is "happy".

A happy number is a number defined by the following process: Starting with any positive integer, replace the number by the sum of the squares of its digits, and repeat the process until the number equals 1 (where it will stay), or it loops endlessly in a cycle which does not include 1. Those numbers for which this process ends in 1 are happy numbers.

Return True if n is a happy number, and False if not.
```
Example: 

Input: 19
Output: true
Explanation: 
12 + 92 = 82
82 + 22 = 68
62 + 82 = 100
12 + 02 + 02 = 1
```

### Solution


```cpp
class Solution {
public:
    bool isHappy(int n) {
        
        // Data structure to check if numbers repeat         
        unordered_map<int,int> map;
        int x = n;
        while(x){
            
            int sum = 0;
            
            // Getting all digits
            while(x){
                int d = x%10;
                x/=10;

                sum+= (d*d);
            }
            
            if(sum == 1) return 1;
            
            // Check if the sum is new or old
            if(map.find(sum)!=map.end()){
                return 0;
            }else{
               // store so that if again there is cycle we can stop
                map[sum] =1;
            }
            x = sum;
        }
               
        return 0;
        
    }
};
```


## 204. Count Primes


### Solution 

Steve

```cpp
class Solution {
public:
    int countPrimes(int n) {
        
        if(n<2) return 0;
        
        int primes [n];
        fill(primes, primes + n, 0);
        int res = 0;
        
        for(int i =2;i<n;i++){
            if(primes[i] == 0) res++;
            for(int j =1;i*j<n;j++){
                primes[i*j] = 1;
            }
        }
        return res;
    }
};
```


## 231. Power of Two

### Solution

```cpp
class Solution {
public:
    bool isPowerOfTwo(int n) {
        if(n < 1) return false;
        int count= 0;
        while(n){
            if(1&n){
                count++;
            }
            n = n>>1; 
            if(count >1) return false;
            
        }
        
        return true;
        
    }
};
```

## 258. Add Digits

Given a non-negative integer num, repeatedly add all its digits until the result has only one digit.
```
Example:

Input: 38
Output: 2 
Explanation: The process is like: 3 + 8 = 11, 1 + 1 = 2. 
             Since 2 has only one digit, return it.
Follow up:
Could you do it without any loop/recursion in O(1) runtime?
```

### Solution

```cpp
class Solution {
public:
    int addDigits(int num) {
        if(num==0) return 0;
        return num % 9 == 0 ? 9 : num % 9;
    }
};

```

## 263. Ugly Number

Write a program to check whether a given number is an ugly number.

Ugly numbers are positive numbers whose prime factors only include 2, 3, 5.

```
Example 1:

Input: 6
Output: true
Explanation: 6 = 2 × 3
Example 2:

Input: 8
Output: true
Explanation: 8 = 2 × 2 × 2
Example 3:

Input: 14
Output: false 
Explanation: 14 is not ugly since it includes another prime factor 7.
```

### Solution

```cpp
class Solution {
public:
    bool isUgly(int num) {
        
        if(num == 0 ) return false;
        while( num%2 == 0 ) num /=2;
        while( num%3 == 0 ) num /=3;
        while( num%5 == 0 ) num /=5;        
        return num == 1;
    }
};
```

## 268. Missing Number

Given an array containing n distinct numbers taken from 0, 1, 2, ..., n, find the one that is missing from the array.

```
Example 1:

Input: [3,0,1]
Output: 2
Example 2:

Input: [9,6,4,2,3,5,7,0,1]
Output: 8
Note:
Your algorithm should run in linear runtime complexity. Could you implement it using only constant extra space complexity?
```

### Solution

```cpp

class Solution {
public:
    int missingNumber(vector<int>& nums) {
        int sm = (nums.size())*(nums.size()+1)/2; 
        for(auto x:nums){
            sm-=x;
        }
        return sm;
    }
    
    int missingNumber(vector<int>& nums) {
        int sm = nums.size();
        for(int i = 0;i<nums.size();i++){
            sm = sm ^ i ^ nums[i]; 
        }
        return sm;
    }
};
```

## 367. Valid Perfect Square

Given a positive integer num, write a function which returns True if num is a perfect square else False.

Follow up: Do not use any built-in library function such as sqrt.

```
Example 1:

Input: num = 16
Output: true
Example 2:

Input: num = 14
Output: false

```

### Solution

```cpp
class Solution {
public:
    bool isPerfectSquare(int num) {
        
        int l = 0,h = num;
        while(l<=h){
            long int m = l + (h-l)/2;
            long sq = m*m;
            if(sq == num) return true;
            if(sq < num) l = m+1;
            else h = m-1;
        }
        
        return false;
    }
};

```
