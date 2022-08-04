#### 编译

```
make	# 编译出所有测试程序
```

#### 使用

```
# 在开发板上放入三个程序：`uart`，`udp`，`ping-pong`。
# 在测试主机上放入程序`master`。

# 串口测试
./uart /dev/ttyS5	# 开发板上执行
./master -s /dev/tty*	# 测试主机上执行

# udp测试
./udp	# 开发板上执行
./master -n <ip>	# 测试主机上执行

# ping-pong测试，udp收串口发
```

