# Redis Note

Check out the following links to continue learning about Redis.

1. [Redis Documentation](http://redis.io/documentation)
1. [Command Reference](http://redis.io/commands)
1. [Implement a Twitter Clone in Redis](http://redis.io/topics/twitter-clone)
1. [Introduction to Redis Data Types](http://redis.io/topics/data-types-intro)

---

As a first example, we can use the command SET to store the value "fido" at key "server:name":


```bash
    SET server:name "fido"
```

Redis will store our data permanently, so we can later ask "What is the value stored at key server:name?" and Redis will reply with "fido":

```bash
    GET server:name => "fido"
```

There is a command in order to test if a give key exists or not:

```bash
    EXISTS server:name => 1
    EXISTS server:blabla => 0
```

## Output

```bash
> SET server:name "fido"
OK
> Get server:name
"fido"
> EXISTS server:name
(integer) 1
> EXISTS server:blalvba
(integer) 0
> NEXT

```

Other basic operations provided by Redis are DEL to delete a given key and associated value, INCR to atomically increment a number stored at a given key:

```bash

    SET connections 10
    INCR connections => 11
    INCR connections => 12
    DEL connections
    INCR connections => 1
```

It is also possible to increment the number contained inside a key by a specific amount:

```bash
    INCRBY connections 100 => 101
```

And there are similar commands in order to decrement the value of the key.

```bash
    DECR connections => 100
    DECRBY connections 10 => 90

```

## Output

```bash
> SET connections 10
OK
> INCR connections
(integer) 11
> INCR connections
(integer) 12
> DEL connections
(integer) 1
> INCR connections
(integer) 1
> INCRBY connections 100
(integer) 101
> DECR connections
(integer) 100
> DECRBY connections 100
(integer) 0

```

## NOTE

---
There is something special about INCR. Why do we provide such an operation if we can do it ourself with a bit of code? After all it is as simple as:

```bash
x = GET count
x = x + 1
SET count x
```

The problem is that doing the increment in this way will only work as long as there is a single client using the key. See what happens if two clients are accessing this key at the same time:

```bash
Client A reads count as 10.
Client B reads count as 10.
Client A increments 10 and sets count to 11.
Client B increments 10 and sets count to 11.
```

We wanted the value to be 12, but instead it is 11! This is because incrementing the value in this way is not an atomic operation. Calling the INCR command in Redis will prevent this from happening, because it is an atomic operation.

All the Redis operations implemented by single commands are atomic, including the ones operating on more complex data structures. So when you use a Redis command that modifies some value, you don't have to think about concurrent access.

---


Redis can be told that a key should only exist for a certain length of time. This is accomplished with the EXPIRE and TTL commands, and by the similar PEXPIRE and PTTL commands that operate using time in milliseconds instead of seconds.

```bash
    SET resource:lock "Redis Demo"
    EXPIRE resource:lock 120
```

This causes the key resource:lock to be deleted in 120 seconds. You can test how long a key will exist with the TTL command. It returns the number of seconds until it will be deleted.

```bash
    TTL resource:lock => 113
    // after 113s
    TTL resource:lock => -2
```

The -2 for the TTL of the key means that the key does not exist (anymore). A -1 for the TTL of the key means that it will never expire. Note that if you SET a key, its TTL will be reset.

```bash

    SET resource:lock "Redis Demo 1"
    EXPIRE resource:lock 120
    TTL resource:lock => 119
    SET resource:lock "Redis Demo 2"
    TTL resource:lock => -1
```

The SET command is actually able to accept further arguments in order to directly set a time to live (TTL) to a key, so you can alter the value of a key and set its TTL at the same time in a single atomic operation:

```bash
    SET resource:lock "Redis Demo 3" EX 5
    TTL resource:lock => 5
```

It is also possible to cancel the time to live of a key removing the expire and making the key permanent again.

```bash
    SET resource:lock "Redis Demo 3" EX 5
    PERSIST resource:lock
    TTL resource:lock => -1
```

Redis also supports several more complex data structures. The first one we'll look at is a list. A list is a series of ordered values. Some of the important commands for interacting with lists are **RPUSH**, **LPUSH**, **LLEN**, **LRANGE**, **LPOP**, and **RPOP**. You can immediately begin working with a key as a list, as long as it doesn't already exist as a different type.

This concept is generally true for every Redis data structure: you don't create a key first, and add things to it later, but you can just directly use the command in order to add new elements. As side effect the key will be create if it did not exist. Similarly keys that will result empty after executing some command will automatically be removed from the key space.

**RPUSH** puts the new element at the end of the list.

```bash
    RPUSH friends "Alice"
    RPUSH friends "Bob"
```

**LPUSH** puts the new element at the start of the list.

```bash
    LPUSH friends "Sam"
```

**LRANGE** gives a subset of the list. It takes the index of the first element you want to retrieve as its first parameter and the index of the last element you want to retrieve as its second parameter. A value of -1 for the second parameter means to retrieve elements until the end of the list, -2 means to include up to the penultimate, and so forth.

```bash
    LRANGE friends 0 -1 => 1) "Sam", 2) "Alice", 3) "Bob"
    LRANGE friends 0 1 => 1) "Sam", 2) "Alice"
    LRANGE friends 1 2 => 1) "Alice", 2) "Bob"

```


So far we explored the commands that let you add elements to the list, and LRANGE that let you inspect ranges of the list. A fundamental functionality of Redis lists is the ability to remove, and return to the client at the same time, elements in the head or the tail of the list.

**LPOP** removes the first element from the list and returns it.

```bash
    LPOP friends => "Sam"
```

**RPOP** removes the last element from the list and returns it.

```bash
    RPOP friends => "3"
```

Note that the list now only has four element:

```bash
    LLEN friends => 4
    LRANGE friends 0 -1 => 1) "Alice" 2) "Bob" 3) "1" 4) "2"
```

Both **RPUSH** and **LPUSH** commands are variadic, so you can specify multiple elements in the same command execution.

```bash
    RPUSH friends 1 2 3 => 6
```

> Tip: RPUSH and LPUSH return the total length of the list after the operation.

You can also use **LLEN** to obtain the current length of the list.

```bash
    LLEN friends => 6
```

The next data structure that we'll look at is a set. A set is similar to a list, except it does not have a specific order and each element may only appear once. Both the data structures are very useful because while in a list is fast to access the elements near the top or the bottom, and the order of the elements is preserved, in a set is very fast to test for membership, that is, to immediately know if a given element was added or not. Moreover in a set a given element can exist only in a single copy.

Some of the important commands in working with sets are **SADD**, **SREM**, **SISMEMBER**, **SMEMBERS** and **SUNION**.

**SADD** adds the given member to the set, again this command is also variadic.

```bash
    SADD superpowers "flight"
    SADD superpowers "x-ray vision" "reflexes"
```

**SREM** removes the given member from the set, returning 1 or 0 to signal if the member was actually there or not.

```bash
    SREM superpowers "reflexes" => 1
    SREM superpowers "making pizza" => 0
```

**SISMEMBER** tests if the given value is in the set. It returns 1 if the value is there and 0 if it is not.

```bash
    SISMEMBER superpowers "flight" => 1
    SISMEMBER superpowers "reflexes" => 0
```

**SMEMBERS** returns a list of all the members of this set.

```bash
    SMEMBERS superpowers => 1) "flight", 2) "x-ray vision"
```


**SUNION** combines two or more sets and returns the list of all elements.

```bash
    SADD birdpowers "pecking"
    SADD birdpowers "flight"
    SUNION superpowers birdpowers => 1) "pecking", 2) "x-ray vision", 3) "flight"
```

The return value of **SADD** is as important as the one of **SREM**. If the element we try to add is already inside, then 0 is returned, otherwise **SADD** returns 1:

```bash
    SADD superpowers "flight" => 0
    SADD superpowers "invisibility" => 1
```

Sets also have a command very similar to **LPOP** and **RPOP** in order to extract elements from the set and return them to the client in a single operation. However since sets are not ordered data structures the returned (and removed) elements are totally casual in this case.

```bash
    SADD letters a b c d e f => 6
    SPOP letters 2 => 1) "c" 2) "a"
```

The argument of **SPOP** after the name of the key, is the number of elements we want it to return, and remove from the set.

Now the set will have just the remaining elements:

```bash

    SMEMBERS letters => 1) "b" 2) "d" 3) "e" 4) "f"
```

There is also a command to return random elements without removing such elements from the set, it is called **SRANDMEMBER**. You can try it yourself, it works just like SPOP, but if you specify a negative count instead of a positive one, it may also return repeating elements.

Sets are a very handy data type, but as they are unsorted they don't work well for a number of problems. This is why Redis 1.2 introduced **Sorted Sets**.

A sorted set is similar to a regular set, but now each value has an associated score. This score is used to sort the elements in the set.

```bash

    ZADD hackers 1940 "Alan Kay"
    ZADD hackers 1906 "Grace Hopper"
    ZADD hackers 1953 "Richard Stallman"
    ZADD hackers 1965 "Yukihiro Matsumoto"
    ZADD hackers 1916 "Claude Shannon"
    ZADD hackers 1969 "Linus Torvalds"
    ZADD hackers 1957 "Sophie Wilson"
    ZADD hackers 1912 "Alan Turing"
```

In these examples, the scores are years of birth and the values are the names of famous hackers.

```bash
    ZRANGE hackers 2 4 => 1) "Claude Shannon", 2) "Alan Kay", 3) "Richard Stallman"
```

Simple strings, sets and sorted sets already get a lot done but there is one more data type Redis can handle: **Hashes.**

Hashes are maps between string fields and string values, so they are the perfect data type to represent objects (eg: A User with a number of fields like name, surname, age, and so forth):

```bash
    HSET user:1000 name "John Smith"
    HSET user:1000 email "john.smith@example.com"
    HSET user:1000 password "s3cret"

```


To get back the saved data use **HGETALL**:

```bash
    HGETALL user:1000

    > HGETALL user:1000
    1) "name"
    2) "john smith"
    3) "email"
    4) "johnsmith@example.com"
    5) "password"
    6) "s3cret"
```

You can also set multiple fields at once:

```bash
    HMSET user:1001 name "Mary Jones" password "hidden" email "mjones@example.com"
```

If you only need a single field value that is possible as well:

```bash
    HGET user:1001 name => "Mary Jones"
```

Numerical values in hash fields are handled exactly the same as in simple strings and there are operations to increment this value in an atomic way.

```bash
    HSET user:1000 visits 10
    HINCRBY user:1000 visits 1 => 11
    HINCRBY user:1000 visits 10 => 21
    HDEL user:1000 visits
    HINCRBY user:1000 visits 1 => 1
```

**[full list of Hash commands](http://redis.io/commands#hash)**

```bash

    HDEL key field [field ...]
    Delete one or more hash fields

    HEXISTS key field
    Determine if a hash field exists
    
    HGET key field
    Get the value of a hash field
    
    HGETALL key
    Get all the fields and values in a hash
    
    HINCRBY key field increment
    Increment the integer value of a hash field by the given number
    
    HINCRBYFLOAT key field increment
    Increment the float value of a hash field by the given amount
    
    HKEYS key
    Get all the fields in a hash
    
    HLEN key
    Get the number of fields in a hash
    
    HMGET key field [field ...]
    Get the values of all the given hash fields
    
    HMSET key field value [field value ...]
    Set multiple hash fields to multiple values
    
    HSET key field value [field value ...]
    Set the string value of a hash field
    
    HSETNX key field value
    Set the value of a hash field, only if the field does not exist
    
    HSTRLEN key field
    Get the length of the value of a hash field
    
    HVALS key
    Get all the values in a hash
    
    HSCAN key cursor [MATCH pattern] [COUNT count]
    Incrementally iterate hash fields and associated values
```