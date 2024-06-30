# CATool

CATool 是一个用 C 语言编写的工具，旨在将多个自定义命令集合到一个易于使用的命令行界面中，同时支持调用系统命令。这个工具为用户提供了一种高效的方法来管理和执行自定义的命令行操作。

## 特性

- 命令集合：将多个自定义命令行命令集合到一个工具中，简化操作。
- 高效执行：使用 C 语言编写，确保高效的性能和快速的执行速度。
- 易于使用：提供简单的命令行界面，方便用户使用。
  - 支持命令行回溯
  - 支持系统命令如pwd、ls等命令调用
  - 快速增加自定义命令

## 教程
### 增加命令
1. 获取本项目
git clone https://github.com/MagicianBoom/CATool.git

2. 实现新命令主入口函数：
```
int new_cmd(int argc, char *argv[])
```
3. 在catool.h头文件的`command commands[]`中加入`new_cmd`
```
{"new_cmd", new_cmd,   "help_info"  },
```

### 编译

你可以通过以下步骤来编译 CATool：

```bash
cd CATool
./build.sh
```
生成的工具为CATool/catool

### 使用方法

编译完成后，你可以通过以下命令来使用 CATool：

```bash
./catool
> help
> hello_world
```

### 修改配置参数
通过修改`catool_conf`而不是`catool_conf.h`文件内容来对参数进行配置。

## 贡献

欢迎贡献代码和提出问题！如果你有任何建议或发现了问题，请提交一个 issue 或创建一个 pull request。

1. Fork 本仓库
2. 创建你的 feature 分支 (`git checkout -b feature/AmazingFeature`)
3. 提交你的更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 打开一个 Pull Request

## 许可证

CATool 采用 [BSD 3-Clause 许可证](LICENSE) 开源。详情请参阅 LICENSE 文件。

## 联系我们

如果你有任何问题或建议，请通过以下方式联系我们：

- 邮件: ly56785@163.com

感谢你对 CATool 的支持和关注！
