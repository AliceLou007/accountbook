# 记账本课程设计

这是一个基于 Qt/C++ 的桌面端记账本程序，支持用户登录、账本管理、收支记录、分类标签、预算提醒、统计图表和多人账本邀请加入。

## 提交材料

- 作业报告：`组号-作业报告.pdf`
- 源代码地址：`组号-源代码.txt`
- 演示视频：待填写北大网盘链接

## 演示视频

待填写：请将 `组号-演示.mp4` 或 `组号-演示.mkv` 上传到北大网盘后，把分享链接粘贴到这里。

## 源代码地址

https://github.com/Alicelou007/accountbook

## 运行说明

使用 Qt Creator 打开 `accountbook.pro` 后构建运行。

多人账本服务器可通过以下方式启动：

```bash
accountbook.exe --server --port 8888
```

客户端可通过以下方式连接服务器：

```bash
accountbook.exe --host 服务器IP --port 8888
```

也可以参考 `server_config.example.json` 创建本地 `server_config.json`。
