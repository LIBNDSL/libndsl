# RDMA pingpong测试报告

## 测试环境
- 系统版本
    Ubuntu 16.04
- RDMA网卡
    Mellanox Technologies MT27500 Family [ConnectX-3]

## 测试说明
- 测试结果的单位均为Mbit/sec，且测试结果统计的数据大小为收发**数据之和**
- -r参数代表一次发布的接收请求个数
- -e参数代表使用事件通知机制
- ibv_rc_pingpong是InfiniBand库自带命令，用于测试RC连接

## 与ibv_rc_pingpong测试结果比较
以buffer为16KB为例

ibv_rc_pingpong测试命令
```
ibv_rc_pingpong -n 1000000 -s 16384 -r 1 -e
ibv_rc_pingpong 10.0.1.1 -n 1000000 -s 16384 -r 1 -e
```

测试结果：
参数|-r 500|-r 1| -r 500 -e | -r 1 -e |
:-:|:-:|:-:|:-:|:-:|
ibv_rc_pingpong|29408|11183| 11185 | 6420 |
rdma_rc_pingpong|X|X| 7775 | 4992 |

## 吞吐量
下表所有测试满足以下条件：
- 一次发布500个接收请求
- 使用事件模式
- 仅改变buffer大小

buffer大小|1KB|4KB|16KB|64KB|128KB|256KB
:-:|:-:|:-:|:-:|:-:|:-:|:-:|
ibv_rc_pingpong|4440|13715|29408|41771|45178|47068|
ibv_rc_pingpong(-e)|1383|3880|11183|24138|31537|37307|
rdma_rc_pingpong|725|2247|7775|19158|25709|32224|


## 结果分析
- 事件模式会导致性能大量降低，可能是因为事件模式的通知机制会在内核损失部分性能，后续可使用DPDK改善