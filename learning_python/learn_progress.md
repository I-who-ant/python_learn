# 学习进度记录

## 2025-10-12
- 任务：重命名项目为 `learning_python`，初始化 README，并建立学习进度记录机制。
- 收获：明确使用 Jupyter Notebook 作为交互式学习笔记，确立仓库维护约定（KISS、YAGNI、DRY）。
- 待办：按指导继续更新 `sample.ipynb` 内容，并在每次学习后补充此文档。
- 追加：重置 Jupyter Server 密码为 `learning_python`，并记录虚拟环境被移动后需重新生成/修复激活脚本。
- 追加：在 `jupyter_server_config.json` 中禁用 token 登录（`ServerApp.token=''`），IDE 必须使用密码登录。
- 追加：同步在 `ServerApp.password` 中写入 `learning_python` 的 Argon2 哈希，确保所有客户端走密码认证。


## 2025-10-13
- 任务：通读并整理 `sample.ipynb`，梳理控制流、函数、文件读写、类与输入输出等示例。
- 收获：
  - 回顾 `for`、`while`、多分支 `if` 的基本语法与 `range` 用法。
  - 理解函数默认参数的顺序限制，并通过 `running_car` 示例演练位置参数与默认参数的组合。
  - 练习文件写入、追加、读取（`read`/`readlines`/遍历文件对象）以及指针重置。
  - 通过 `Car` 类示例复习 `__init__`、实例属性、实例方法以及简单的字符串拼接输出。
  - 演示 `input()` 返回字符串以及遍历元组、列表的索引访问方式。
  - 已将 Notebook 中全部 `//` 注释替换为合法的 `#` 注释，确保代码单元可执行（KISS）。
- 待办：
  - 修正 `running_car` 的函数签名示例，去除冒犯性占位词并确保默认参数位于无默认参数之后（SOLID-S）。
  - 为 `Car` 类保留单一的有效构造函数实现，并补全未完成的 `__init__` 定义（DRY）。


