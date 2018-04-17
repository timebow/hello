# 说明
  参考： https://blog.csdn.net/aobai219/article/details/1596964
  分别由一个Host和一个clint程序, 用于测试sock发送接收.
  1. HOST: 代码实例中的服务器通过socket连接向客户端发送字符串"Hello, you are connected!"。
     只要在服务器上运行该服务器软件，在客户端运行客户软件，客户端就会收到该字符串。
  2. CLIENT: 客户端程序首先通过服务器域名获得服务器的IP地址，然后创建一个socket，调用connect函数与服务器建立连接，
     连接成功之后接收从服务器发送过来的数据，最后关闭socket。
     函数gethostbyname()是完成域名转换的。由于IP地址难以记忆和读写，所以为了方便，人们常常用域名来表示主机，
     这就需要进行域名和IP地址的转换。

# 编译
  分别进入host/client编译, 会生成host/client程序.

# 运行
  分别在两台机器上运行Host和client. client目前通过域名直接解析IP, 需要输入域名给client.

