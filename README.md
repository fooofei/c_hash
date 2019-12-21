# c_hash

## 这个 hash 的思路起源一个讨论

[上]https://mp.weixin.qq.com/s?__biz=MzAxMzYwMDE4OA==&mid=206305512&idx=1&sn=58f3b49079f634e2df6820e11c9b56fb&scene=4#wechat_redirect

[下]https://mp.weixin.qq.com/s?__biz=MzAxMzYwMDE4OA==&mid=206303795&idx=1&sn=d5f76d2832e909d81e535a9ca27f85cd&scene=1&key=af154fdc40fed003e56336356c9a33a693767855ab05255ff386bc3cc90bd40bd48e05ca5d6bdfc3a9ebfd4bbec17876&ascene=1&uin=MzY0NTg3NDgw&devicetype=webwx&version=70000001&pass_ticket=7Lgf2OjZJ6QcvvrNWfsOdLfCiG%2FoZFZ%2F%2FbLw9CR1ZnOLxUDIoai1RJp5vtAvT6i%2B

[老外的讨论]http://d.hatena.ne.jp/fcicq/touch/

龙博：这是最核心的，说出来其实也很简单。我在总结一下，最关键的几部分：
1. 开放地址探测的哈希（非链式哈希！）
2. 哈希表的key值压缩在一个8字以内
3. 稍微特别考虑一下哈希表的删除（你得维护key冲突的时候的链条）

龙博：实际测试结果，一亿条记录，在哈希表的装载率（load factor）为70%的情况下，平均查询次数为1.1次，最大查询次数不超过4次。。这应该是最好的结果了。key+value 共16字节。这应该是最高效的存储结构了。
用开放地址探测的散列，要保证在一定步数内找到一个空位，你必须让数组长度为一个质数。在装载率为70%的情况下，一个质数长度的数组，基本就是几步之内你就一定能找到一个空位。

开放地址探测 key 为uint64_t 有读写判断

上面说的开放地址探测，只是排除了不使用开链法。但却没区分开开放地址探测在中的二次探测还是线性探测。


新增操作是 找到尾部然后添加到尾部
删除操作是 如果是尾部，就把上面的一个置成尾部 如果不是尾部就把尾部复制过来，把尾部的上一个置成尾部
 



[C# implements] https://github.com/daleiyang/LockFreeHashTable
[Java implements] http://zhaobohao.iteye.com/blog/2258098

[good hash table primes] http://planetmath.org/goodhashtableprimes
```
我取值 2^17	2^18	0.002543	196613 比较合适
```

[一个高性能无锁哈希表的实现] https://blog.csdn.net/divfor/article/details/44316291 https://github.com/divfor/atomic_hash/blob/master/src/atomic_hash.c
这个写的太复杂了 没办法移植到项目里

[如何设计并实现一个线程安全的 Map ？(上篇) halfrost.com](https://halfrost.com/go_map_chapter_one/)
对比 Java 和 GO 语言

[Lockfree Hashtable spiritsaway.info](http://spiritsaway.info/lockfree-hashtable.html)
Get 被他搞成了 O(n) 的了，这一一点也没有 hash 的作用了。


我的疑问：

1 使用开放地址法，在处理冲突的时候，是按照顺序下移索引，这不就是链表吗？跟用链表解决冲突没有区别。

    其实比用链表，查找元素更复杂了，因为这个开放地址法是交叉的链表。
    顺序遍历桶的过程中，如何判断一个桶的元素是不是我们当前链表的呢，就看 KEY 是不是一样。
    这就引起问题了，如果一个桶的 KEY 相同的有 5 个，他向后依次找了 5 个位置，那如果查询的时候，
    正好找到他后面 5 个位置中的 1 个，那么判断 KEY 不相等，是还继续向后看吗，
    没有机制判断向后看到什么时候是结束，那这样找到什么时候是个头。
    是 KEY=0 就是个头？这样合计感觉不是个通用的无锁哈希表。
    这样看差异，还会造成不同线程访问同一个链，因为是从不同位置切入的.

2 在一定步数内找到一个空位是怎么达成的？

    寄希望于相邻被占用的桶比较稀散 空位多，跟 70% 的概念沾边了，10 个里面，有 3 个空桶

3 当使用开放地址法的线性探测法时，如果存在插入序列 A0,A1,A2,B0,A3 （A0 A1 A2 A3 为 hash key 相同），插入位置如图

    |---A0-A1-A2-B0-A3---------------|

    在这种条件下，A3 的插入，把 A2 是尾部变成不是尾部，A3 是尾部。B0 是尾部，
    他俩 KEY 不一样，在不同的链表上。

3 二次探测法对无锁是否友好？

另一个 hash 函数
```c
static inline unsigned hashkey(unsigned int dest_ip) 
{ 
  return (dest_ip* 2654435761UL) & HASH_TAB_MASK; 
} 
```
其中，2654435761UL是2到2^32 (4294967296)间接近于黄金分割的素数， 
(sqrt(5) - 1) / 2 = 0.618033989 
2654435761 / 4294967296 = 0.618033987


[最好的 hash 函数 murmur3](https://github.com/PeterScott/murmur3)

[各种的 hash 测试 smhasher](https://github.com/demerphq/smhasher)

[Maglev: A Google Maglev Hashing Algorithm implement in Golang](https://github.com/kkdai/maglev)

[Extremely fast non-cryptographic hash algorithm](https://github.com/Cyan4973/xxHash)

[python用libcfu实现hash](https://github.com/python/cpython/blob/master/Modules/hashtable.c)

[Looking for a good hash table implementation in C](https://stackoverflow.com/questions/1138742/looking-for-a-good-hash-table-implementation-in-c)

