# c_hash

[上]https://mp.weixin.qq.com/s?__biz=MzAxMzYwMDE4OA==&mid=206305512&idx=1&sn=58f3b49079f634e2df6820e11c9b56fb&scene=4#wechat_redirect

[下]https://mp.weixin.qq.com/s?__biz=MzAxMzYwMDE4OA==&mid=206303795&idx=1&sn=d5f76d2832e909d81e535a9ca27f85cd&scene=1&key=af154fdc40fed003e56336356c9a33a693767855ab05255ff386bc3cc90bd40bd48e05ca5d6bdfc3a9ebfd4bbec17876&ascene=1&uin=MzY0NTg3NDgw&devicetype=webwx&version=70000001&pass_ticket=7Lgf2OjZJ6QcvvrNWfsOdLfCiG%2FoZFZ%2F%2FbLw9CR1ZnOLxUDIoai1RJp5vtAvT6i%2B

[老外的讨论]http://d.hatena.ne.jp/fcicq/touch/

开放地址探测 key 为uint64_t 有读写判断



龙博：这是最核心的，说出来其实也很简单。我在总结一下，最关键的几部分：
1. 开放地址探测的哈希（非链式哈希！）
2. 哈希表的key值压缩在一个8字以内
3. 稍微特别考虑一下哈希表的删除（你得维护key冲突的时候的链条）

龙博：实际测试结果，一亿条记录，在哈希表的装载率（load factor）为70%的情况下，平均查询次数为1.1次，最大查询次数不超过4次。。这应该是最好的结果了。key+value 共16字节。这应该是最高效的存储结构了。
用开放地址探测的散列，要保证在一定步数内找到一个空位，你必须让数组长度为一个质数。在装载率为70%的情况下，一个质数长度的数组，基本就是几步之内你就一定能找到一个空位。




[C# implements] https://github.com/daleiyang/LockFreeHashTable
[Java implements] http://zhaobohao.iteye.com/blog/2258098

[good hash table primes] http://planetmath.org/goodhashtableprimes
```
我取值 2^17	2^18	0.002543	196613 比较合适
```

[一个高性能无锁哈希表的实现] https://blog.csdn.net/divfor/article/details/44316291 https://github.com/divfor/atomic_hash/blob/master/src/atomic_hash.c
这个写的太复杂了 没办法移植到项目里

[如何设计并实现一个线程安全的 Map ？(上篇)] https://halfrost.com/go_map_chapter_one/
对比 Java 和 GO 语言

[Lockfree Hashtable] http://spiritsaway.info/lockfree-hashtable.html

我的疑问：

1 使用开放地址法，在处理冲突的时候，是按照顺序下移索引，这不就是链表吗？跟用链表解决冲突没有区别。

在一定步数内找到一个空位是怎么达成的？

2 当使用开放地址法的线性探测法时，如果存在插入序列 A0,A1,A2,B0,A3 （A0 A1 A2 A3 为 hash key 相同），插入位置如图

|---A0-A1-A2-B0-A3---------------|

在这种条件下，A3 的插入非常不合适。

3 二次探测法对无锁是否友好？

另一个 hash key 计算方式

    static inline unsigned hashkey(unsigned int dest_ip) 
　　{ 
　　  return (dest_ip* 2654435761UL) & HASH_TAB_MASK; 
　　} 
　　其中，2654435761UL是2到2^32 (4294967296)间接近于黄金分割的素数， 
　　(sqrt(5) - 1) / 2 = 0.618033989 
　　2654435761 / 4294967296 = 0.618033987
