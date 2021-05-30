# Fight landlord 2

程设带作业

守护全世界最可爱的勤健哥哥！

## 封装

```c++
/*
  map: [0, 15] -> {3456789TJQKA_2mM}
  第 12 位被留作空位，是为了确保后面的2和大小王不会作为顺子出现，从而避免繁琐的特判
*/

// 以下定义一些全局变量
int MyPosition, LordPosition, LordBid;
std::vector<int>bidInfo;
std::set<int>MyHand, PublicHand;
std::vector<std::vector<int>>GameHistory[3];
Combo lastCombo;
int remained_cnt[3] = {17, 17, 17};
int remained_card[16] = {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 0, 4, 1, 1};
int what_to_do;// BIDDING or PLAYING
// 以上是一些全局变量

class Partition{
    std::vector<int>single[4];// 广义单牌，依次表示单张、对子、三连、炸弹
    std::vector<PIR>straight[4];// 广义顺子，依次表示单顺、双顺、飞机、航天飞机 pair标注起止位置(闭)
    /*
     blabla...
    */
};
class Hands{
	int cnt[16];
    std::vector<int>card[16];
    std::vector<Partition>Partitions;
    /*
     blabla...
    */
};
class Combo{// 内部实现的一个类，大可以置之不管
    int k, l, r, w, c;// [l, r]中的每张牌各k张，带的翼大小为 w ，个数为 c
}
```

实现逻辑大致是写了两个 `namespace`

```c++
namespace BIDDING{
    int solve(){
        return // 叫几分
    }
}
namespace PLAYING{
    std::vector<int> solve(){
        return // 打什么牌，注意这里的 std::vector 里存的是牌的编号
    }
}
```

`PLAYING::solve()` 的大致实现是枚举所有能够打出的牌型，剩下的手牌构造一个 `Hands` 对象，通过比较 `Hands::eval()` 的大小，保留一个最大的作为返回结果。

`Hands::eval()` 的实现方式是

```c++
int Hands::eval(){
	Try2Partition();// 捜出所有的 Partition 并存在 Partitions 里
	int res = 0;
	for(Partition p: Partitions)
	    res = std::max(res, p.eval());
	return res;
}
```

这里需要自己实现 `Partition::eval()` 。当然要改 `Hands::eval()` 也成。



## run.cpp 是啥

（在李佳衡先生的协助下）实现了一个本地模拟 Botzone 环境的轮子。

编译以后调用 `run.exe bot1name bot2name bot3name [games]` 即可， `games` 可以不写，默认为$1$

~~会实现一个单步调试功能。~~ gugugu



## eval 怎么写

gugugu

### version 0.0

```c++
int Partition::eval(){
    static int val[2][4] = {
		{1, 10, 50, 5000},
		{200, 500, 1000, 10000}
    };
    int res = 0;
    for(int i = 0; i < 4; ++i){
		for(int x: single[i])
		   	res += x * val[0][i];
		for(PIR x: straight[i])
	    	res += (x.first + x.second) * val[1][i];
    }
    return res;
}
```

这个估价他非常的不合理，实践验证发现他指挥出单牌（因为这样就可以保留最高的 value）



## 碎碎念

### 5-13 凌晨 的版本：

  今天下午提到过说，选择是否拆散对子会导致划分数指数爆炸（虽然也不会太大），所以打算作一个额外限制：不妨认为相同单牌必须合成对，相同单牌和对子必须合成三连，同时所有三连、飞机、炸弹等默认不匹配带的东西（实际打出时再从不带/带单/带对中做决策）
  前面说的那个不妨应该是有道理的，个人觉得在估价函数中这种偏序关系应该要被体现出来，即两倍单张的权值和不应该高于一对的权值，单张+一对的权值不应该高于三连的权值。
  先这么写看看状态数能有多大。
  17张牌的时候平均状态数10，20张牌的时候平均状态数27。
  今天先就这样吧，睡了qwq

  TO-DO: 把 Combo 写了，然后实现一下出牌（找一个合法的牌型打出去，剩下的再去做划分）

### 5-13 的版本：

​	写完 Combo ，再实现一下暴力枚举出什么牌，剩下的部分作划分然后估价。

​	我感觉就算是直接暴力枚举所有可能的出牌牌型（比如说飞机带一对直接暴力枚举带什么）状态数也不会太多？

​	打算再尝试调一调 BotzoneIO ，争取能在 Botzone 上跑起来。（没写完）

### 5-14 的版本：

​	鸽子终于又来干活了qwq

​	实现了`run.cpp`可以在本地自己跑bot啦（好慢啊跑一局要$12s$左右，有一点点拉

​	编译出`run.exe`后运行`run.exe bot1name bot2name bot3name [games]`即可，`games`可以不写，默认为$1$

​	然后发现`landlord.cpp`可能有点bug，肝不动了明天调把	

### 5-18 的版本：

​	更新了 readme .

### 5-22：

​	[开局王炸，好文明！](https://www.botzone.org.cn/match/60a7f5bb9a1cd711d261e4d0)

​	改了一些内部实现。

​	明天形式化的写一下之前说到的“基于先手数的估价”吧。	

