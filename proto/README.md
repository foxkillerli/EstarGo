## ICE proto   
输入结构体
```
sequence<string> RequestBatch;
```
按batch给神经网络发送当前盘面  
盘面编码为一串字符串，第一个字母表示横坐标，第二个字母表示纵坐标，如：
```plain
AaBbCcDdEeFf
//对应sgf坐标，注意横坐标跳过I。横坐标大写表示
```
batch_size为1、4、8，根据conf设置

输出结构体
```
struct MoveProb{
    pos: int; //棋盘坐标 x * 19 + y
    prob: float; //每个坐标的policy概率
}
```
Policy Reply是长度为361的MoveProb类型数组

```
struct Reply{
    moves: [MoveProb; 361];
    v: float; // 当前盘面value的输出
}
```
ReplyBatch为batch_size * Reply结构体

ICE的调用Interface为evaluate函数