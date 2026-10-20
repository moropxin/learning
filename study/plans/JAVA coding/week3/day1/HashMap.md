# 后端agent学习计划 — HashMap 入门知识概念

## 今日目标
- 学习 HashMap 的基本概念与使用场景
- 理解底层结构、put/get 流程、哈希冲突与扩容
- 完成基础代码练习与 2 道 Map 计数题
- 记录复盘问题与学习小结，待后续 push 后自行回答

---

## 一、HashMap 概念快速入门

### 1. HashMap 是什么
HashMap 是 Java 集合框架中的一种常用映射（Map）实现，用来存储 key-value 对。

它的核心作用是：
- 把一个 key 映射到一个 value
- 之后可以通过 key 直接查到对应 value

例如：
- 学生姓名 -> 年龄
- 课程名 -> 分数
- 字符串 -> 出现次数

常见写法：
```java
Map<String, Integer> map = new HashMap<>();
map.put("Tom", 90);
map.put("Jerry", 85);
System.out.println(map.get("Tom")); // 90
```

### 2. HashMap 底层结构
HashMap 的底层本质可以理解为：
- 一个数组
- 每个数组位置叫作“桶”
- 桶里可以存放链表或红黑树（JDK 8 及之后）

因此，HashMap 的结构可以概括为：
- 数组 + 链表/红黑树

### 3. 为什么叫 HashMap
因为它使用哈希（Hash）算法来决定元素放到数组的哪个位置。

简单理解：
- key 会先计算 hash 值
- hash 值再决定它落在数组的哪个桶中
- 这样在查找时也能快速定位到位置

### 4. put() 的过程
当执行 `map.put(key, value)` 时，HashMap 大致会做这几步：
1. 计算 key 的 hash 值
2. 根据 hash 值确定数组位置（桶下标）
3. 如果该位置为空，直接放入
4. 如果该位置已经有元素：
   - 检查 key 是否相同
   - 相同则覆盖 value
   - 不同则发生哈希冲突
5. 冲突时，采用链表/红黑树继续存放

### 5. get() 的过程
当执行 `map.get(key)` 时：
1. 计算 key 的 hash 值
2. 根据 hash 值找到桶的位置
3. 在桶中比较 key 是否相同
4. 如果相同，则返回 value
5. 如果不同，则继续沿链表或树往后查找

### 6. 哈希冲突是什么
如果两个 key 的 hash 值不同，但计算后映射到同一个数组位置，就称为哈希冲突。

例如：
- key1 和 key2 在逻辑上不同
- 但经过 hash 运算后，落在同一个桶中
- 这时需要用链表或红黑树来解决

### 7. 扩容机制
HashMap 在元素越来越多时，会翻倍扩容。

默认：
- 初始容量：16
- 负载因子：0.75
- 当元素数量达到 16 × 0.75 = 12 时，开始扩容

扩容的意义：
- 减少哈希冲突
- 提高查找效率
- 保持整体性能更稳定

### 8. HashMap 的主要特点
- key 唯一：重复 key 会覆盖旧 value
- key 可以为 null（仅允许一个 null key）
- value 可以重复
- 不保证顺序
- 线程不安全

### 9. HashMap 与 Hashtable / TreeMap 的区别
- HashMap：高效、线程不安全、允许 null
- Hashtable：线程安全、效率较低、不能存 null
- TreeMap：按 key 排序，适合有序场景

### 10. HashMap 的学习重点

- HashMap 用来存 key-value
- 底层是数组 + 链表/红黑树
- 通过 hash 定位桶
- 冲突时使用链表或树处理
- put/get 都依赖 key 的 hash 和 equals
- 扩容是为了降低冲突，提高效率

---

## 二、HashMap 理解要点记录


### 理解要点 1：HashMap 是什么
- 我的理解：一个用hash算法和数组以及键值key为架构支撑的快查表

### 理解要点 2：HashMap 的底层结构
- 我的理解：本质用数组存放map，用key查找value - ‘桶’，当出现key值不同而value相同时，会发生哈希冲突，用链表进行分支（一个桶装多个key对应的value），当数据超过8个时，hashmap会自动变为红黑树结构。
- 关键字：数组 / 桶 / 链表 / 红黑树

### 理解要点 3：为什么能快速查找
- 我的理解：通过hash算法用key值快速计算得出对应下标，在数组中一次定位得到value
- 关键字：hash / 下标 / 定位

### 理解要点 4：put/get 原理
- 我的理解：通过hash算法快速找到对应地址对其进行修改
- put 流程：map.put(key,value); [放入数据]
- get 流程：map.get(key);       [访问数据]

### 理解要点 5：哈希冲突
- 我的理解：不同key值通过hash算法计算得到的键值相同，产生哈希冲突
- 冲突的表现：key不同而value相同
- 解决方式：生成链表同时存放

### 理解要点 6：扩容
- 我的理解：数据较多，防止运行效率降低，开辟空间保证效率
- 默认容量与负载因子：容量-[16] 负载因子-[0.75]
- 什么时候扩容：元素数量超过[16 * 0,75]

### 理解要点 7：HashMap 的注意事项
- key 是否重复：唯一
- null 是否允许：至多一个
- 线程安全：较不安全

---

#代码练习
目标：掌握 HashMap 的最基础使用方式

请完成下面的 Java 代码结构：
- 创建 HashMap
- 存入 key-value
- 使用 get() 获取值
- 处理重复 key 覆盖
- 遍历 keySet() / entrySet()

#### 代码
```java
import java.util.HashMap;
import java.util.Map;

public class test1 {
    public static void main(String[] args) {
        Map<String,Integer> map = new HashMap<>(); //基本数据类型要用包装类
        map.put("zhangsna",18);
        map.put("lisi",19);
        map.put("zhangsan",20);

        //统计单词出现次数
        Map<String,Integer> map3 = new HashMap<>();
        for(String key:map.keySet()){
            if(map3.containsValue(map.get(key))){
                int num = map3.get(key);
                map3.put(key,num+1);
            }else {
                map3.put(key,1);
            }
        }
        for(String key:map.keySet()){
            System.out.println(key+" 出现次数为:"+map3.get(key));
        }


        //统计字符串出现次数
        Map<Character,Integer> map2 = new HashMap<>();
        for(String key:map.keySet()){
            for(int i=0;i<key.length();i++) {
                char c = key.charAt(i);
                if (c == ' ') {
                    continue;
                } else if (map2.containsKey(c)) {
                    int num = map2.get(c);
                    map2.put(c, num + 1);
                } else {
                    map2.put(c, 1);
                }
            }
        }

        for(Map.Entry<Character,Integer> entry:map2.entrySet()){
            System.out.println("字符:"+entry.getKey()+" 对应次数："+entry.getValue());
        }

        //查询张三是否存在
        if(map.get("zhangsan").equals(18)){
            System.out.println("18岁的张三存在");
        } else if (map.get("zhangsan").equals(20)) {
            System.out.println("20岁的张三存在");
        }else {
            System.out.println("不存在");
        }

    }
}public class test1 {
    public static void main(String[] args) {
        Map<String,Integer> map = new HashMap<>();
        map.put("zhangsna",18);
        map.put("lisi",19);
        map.put("zhangsan",20);

        //统计单词出现次数
        Map<String,Integer> map3 = new HashMap<>();
        for(String key:map.keySet()){
            if(map3.containsValue(map.get(key))){
                int num = map3.get(key);
                map3.put(key,num+1);
            }else {
                map3.put(key,1);
            }
        }
        for(String key:map.keySet()){
            System.out.println(key+" 出现次数为:"+map3.get(key));
        }


        //统计字符串出现次数
        Map<Character,Integer> map2 = new HashMap<>();
        for(String key:map.keySet()){
            for(int i=0;i<key.length();i++) {
                char c = key.charAt(i);
                if (c == ' ') {
                    continue;
                } else if (map2.containsKey(c)) {
                    int num = map2.get(c);
                    map2.put(c, num + 1);
                } else {
                    map2.put(c, 1);
                }
            }
        }

        for(Map.Entry<Character,Integer> entry:map2.entrySet()){
            System.out.println("字符:"+entry.getKey()+" 对应次数："+entry.getValue());
        }

        //查询张三是否存在
        if(map.get("zhangsan").equals(18)){
            System.out.println("18岁的张三存在");
        } else if (map.get("zhangsan").equals(20)) {
            System.out.println("20岁的张三存在");
        }else {
            System.out.println("不存在");
        }

    }
}
```

### 2. 蓝桥杯真题 2 道

- 蓝桥杯真题 1：待补充
- 蓝桥杯真题 2：待补充

### 3. 复盘与口头问答


#### 复盘问题清单
- [ ] HashMap 是什么？
- [ ] 它的底层结构是什么？
- [ ] put() 的流程是什么？
- [ ] get() 的流程是什么？
- [ ] 什么是哈希冲突？
- [ ] 为什么会扩容？
- [ ] HashMap 和 Hashtable 的区别是什么？
- [ ] HashMap 为什么适合做计数题？

---
