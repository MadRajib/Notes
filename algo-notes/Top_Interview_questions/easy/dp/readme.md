## 53. Maximum Subarray

Given an integer array nums, find the contiguous subarray (containing at least one number) which has the largest sum and return its sum.

```
Example:

Input: [-2,1,-3,4,-1,2,1,-5,4],
Output: 6
Explanation: [4,-1,2,1] has the largest sum = 6.
Follow up:

If you have figured out the O(n) solution, try coding another solution using the divide and conquer approach, which is more subtle.
```
### Solution

```cpp
class Solution {
public:
    int maxSubArray(vector<int>& nums) {
        int ans=nums[0],i,sum=0;
        for(auto x:nums){
            sum+=x;
            ans=max(sum,ans);
            sum=max(sum,0);
        }
        return ans;
    }
};

```

## 70. Climbing Stairs

You are climbing a stair case. It takes n steps to reach to the top.

Each time you can either climb 1 or 2 steps. In how many distinct ways can you climb to the top?

Note: Given n will be a positive integer.

```
Example 1:

Input: 2
Output: 2
Explanation: There are two ways to climb to the top.
1. 1 step + 1 step
2. 2 steps
Example 2:

Input: 3
Output: 3
Explanation: There are three ways to climb to the top.
1. 1 step + 1 step + 1 step
2. 1 step + 2 steps
3. 2 steps + 1 step
```

### Solution

```cpp
class Solution {
public:
    int climbStairs(int n) {
        int res = 1;
        int a = 1;
        int b = 1;
        for(int i = 2;i<=n;i++){
            res = a+b;
            a = b;
            b = res;
        }
        return res;
    }
};
```