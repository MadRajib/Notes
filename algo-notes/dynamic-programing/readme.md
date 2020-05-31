## 983. Minimum Cost For Tickets

In a country popular for train travel, you have planned some train travelling one year in advance.  The days of the year that you will travel is given as an array days.  Each day is an integer from 1 to 365.

Train tickets are sold in 3 different ways:

a 1-day pass is sold for costs[0] dollars;
a 7-day pass is sold for costs[1] dollars;
a 30-day pass is sold for costs[2] dollars.
The passes allow that many days of consecutive travel.  For example, if we get a 7-day pass on day 2, then we can travel for 7 days: day 2, 3, 4, 5, 6, 7, and 8.

Return the minimum number of dollars you need to travel every day in the given list of days.

 
```
Example 1:

Input: days = [1,4,6,7,8,20], costs = [2,7,15]
Output: 11
Explanation: 
For example, here is one way to buy passes that lets you travel your travel plan:
On day 1, you bought a 1-day pass for costs[0] = $2, which covered day 1.
On day 3, you bought a 7-day pass for costs[1] = $7, which covered days 3, 4, ..., 9.
On day 20, you bought a 1-day pass for costs[0] = $2, which covered day 20.
In total you spent $11 and covered all the days of your travel.
Example 2:

Input: days = [1,2,3,4,5,6,7,8,9,10,30,31], costs = [2,7,15]
Output: 17
Explanation: 
For example, here is one way to buy passes that lets you travel your travel plan:
On day 1, you bought a 30-day pass for costs[2] = $15 which covered days 1, 2, ..., 30.
On day 31, you bought a 1-day pass for costs[0] = $2 which covered day 31.
In total you spent $17 and covered all the days of your travel.
```

Note:

1 <= days.length <= 365
1 <= days[i] <= 365
days is in strictly increasing order.
costs.length == 3
1 <= costs[i] <= 1000

### Solution
Intuition
For each travel day, we can buy a one-day ticket, or use 7-day or 30-day passes as if we would have purchased it 7 or 30 days ago. We need to track rolling costs for at least 30 days back, and use them to pick the cheapest option for the next travel day.
Here, we can use two approaches: track cost for all calendar days, or process only travel days. The first approach is simpler to implement, but it's slower. Since the problem is limited to one calendar year, it does not make much of a difference; for a generalized problem I would recommend the second approach.

1. Track calendar days
We track the minimum cost for all calendar days in dp. For non-travel days, the cost stays the same as for the previous day. For travel days, it's a minimum of yesterday's cost plus single-day ticket, or cost for 8 days ago plus 7-day pass, or cost 31 days ago plus 30-day pass.

```cpp
class Solution {
public:
    int mincostTickets(vector<int>& days, vector<int>& costs) {
        unordered_set<int> dates(days.begin(),days.end());
        vector<int> dp(366,0);
        for(int i =1;i<dp.size();i++){
            if(dates.find(i) == dates.end()) dp[i] = dp[i-1];
            else dp[i] = min({dp[i-1] + costs[0], 
                        dp[max(0,i-7)] + costs[1],
                        dp[max(0,i-30)] + costs[2]});
        }
        
        return dp.back();
    }
};

```

2. Track travel days
We track the minimum cost for each travel day. We process only travel days and store {day, cost} for 7-and 30-day passes in the last7 and last30 queues. After a pass 'expires', we remove it from the queue. This way, our queues only contains travel days for the last 7 and 30 days, and the cheapest pass prices are in the front of the queues.

```cpp
class Solution {
public:
    int mincostTickets(vector<int>& days, vector<int>& costs,int cost =0) {
        queue<pair<int,int>> last7,last30;
        for(auto d: days){
            
            while(!last7.empty() && last7.front().first +7 <= d){
                //pop untill the first day in the queue is more than 7 days old
                last7.pop();
            }
            
            while(!last30.empty() && last30.front().first +30 <= d){
                //pop untill the first day in the queue is more than 30 days old
                last30.pop();
            }
            last7.push({d,cost+costs[1]});
            last30.push({d,cost+costs[2]});
            
            cost = min({cost + costs[0] , 
                        last7.front().second ,
                        last30.front().second});
            
        }
        
        return cost;
    }
};
```