# Agent开发完整技术栈指南

## 您的Agent之路

### 为什么选择Agent开发?

```
您的技术背景:
✅ JavaWeb - 后端服务能力
✅ Python - AI/ML基础
✅ Flask - 轻量级Web
✅ CLI工具 - 用户交互
✅ Linux - 开发环境
✅ MCP协议 - Agent通信

完美契合Agent开发!
```

---

## Agent技术栈全景图

```
┌────────────────────────────────────────────────────┐
│                   用户层                            │
│  CLI界面 / Web界面 / API接口 / 集成应用             │
└────────────────────────────────────────────────────┘
                        ↓
┌────────────────────────────────────────────────────┐
│                 Agent应用层                         │
│  对话Agent / 任务Agent / 协作Agent / 自主Agent      │
└────────────────────────────────────────────────────┘
                        ↓
┌────────────────────────────────────────────────────┐
│                 Agent框架层                         │
│  LangChain / LlamaIndex / AutoGen / CrewAI         │
└────────────────────────────────────────────────────┘
                        ↓
┌────────────────────────────────────────────────────┐
│                  协议/工具层                        │
│  MCP / Function Calling / Tool Use / Memory        │
└────────────────────────────────────────────────────┘
                        ↓
┌────────────────────────────────────────────────────┐
│                   AI模型层                          │
│  LLM / Embedding / Vector DB / RAG                 │
└────────────────────────────────────────────────────┘
                        ↓
┌────────────────────────────────────────────────────┐
│                  基础设施层                         │
│  Python / Rust / Java / Docker / K8s               │
└────────────────────────────────────────────────────┘
```

---

## 第一部分:核心框架选择

### 1. LangChain - 最流行 ⭐⭐⭐⭐⭐

#### 简介
```
定位: 全功能Agent开发框架
语言: Python / TypeScript
特点: 生态最成熟、组件丰富
适合: 快速开发、完整应用
```

#### 核心组件

```python
from langchain.agents import AgentExecutor, create_openai_tools_agent
from langchain.tools import Tool
from langchain_openai import ChatOpenAI
from langchain import hub

# 1. 定义工具
def search_tool(query: str) -> str:
    """搜索工具"""
    return f"Search results for: {query}"

tools = [
    Tool(
        name="Search",
        func=search_tool,
        description="搜索互联网信息"
    )
]

# 2. 初始化模型
llm = ChatOpenAI(model="gpt-4", temperature=0)

# 3. 创建Agent
prompt = hub.pull("hwchase17/openai-tools-agent")
agent = create_openai_tools_agent(llm, tools, prompt)

# 4. 执行器
agent_executor = AgentExecutor(
    agent=agent,
    tools=tools,
    verbose=True
)

# 5. 运行
result = agent_executor.invoke({
    "input": "搜索最新的AI新闻"
})
```

#### 主要模块

```
LangChain核心模块:

1. Models (模型)
   ├── LLMs (大语言模型)
   ├── Chat Models (聊天模型)
   └── Embeddings (向量化)

2. Prompts (提示)
   ├── Prompt Templates
   ├── Few-shot Examples
   └── Output Parsers

3. Chains (链)
   ├── LLMChain (基础链)
   ├── Sequential Chain (顺序链)
   └── Router Chain (路由链)

4. Agents (智能体)
   ├── ReAct Agent
   ├── OpenAI Functions Agent
   └── Custom Agent

5. Memory (记忆)
   ├── Conversation Buffer
   ├── Summary Memory
   └── Vector Store Memory

6. Tools (工具)
   ├── 内置工具(搜索、计算等)
   ├── 自定义工具
   └── Tool Calling

7. Retrievers (检索)
   ├── Vector Store
   ├── BM25
   └── Hybrid Search
```

#### 实战示例:完整的RAG Agent

```python
from langchain.embeddings import OpenAIEmbeddings
from langchain.vectorstores import Chroma
from langchain.text_splitter import RecursiveCharacterTextSplitter
from langchain.chains import RetrievalQA
from langchain.document_loaders import TextLoader

# 1. 加载文档
loader = TextLoader("docs.txt")
documents = loader.load()

# 2. 分割文档
text_splitter = RecursiveCharacterTextSplitter(
    chunk_size=1000,
    chunk_overlap=200
)
splits = text_splitter.split_documents(documents)

# 3. 向量化存储
vectorstore = Chroma.from_documents(
    documents=splits,
    embedding=OpenAIEmbeddings()
)

# 4. 创建检索器
retriever = vectorstore.as_retriever(
    search_type="similarity",
    search_kwargs={"k": 3} #  返回3个最相关的文档
)

# 5. 创建QA链
qa_chain = RetrievalQA.from_chain_type(
    llm=ChatOpenAI(model="gpt-4"),
    chain_type="stuff",
    retriever=retriever,
    return_source_documents=True
)

# 6. 查询
result = qa_chain({"query": "什么是Agent?"})
print(result['result'])
```

---

### 2. LlamaIndex - 数据驱动 ⭐⭐⭐⭐

#### 简介
```
定位: 数据增强的Agent框架
特点: RAG强大、数据索引优秀
适合: 知识库问答、文档分析
```

#### 核心概念

```python
from llama_index import VectorStoreIndex, SimpleDirectoryReader
from llama_index.node_parser import SimpleNodeParser
from llama_index.llms import OpenAI

# 1. 加载数据
documents = SimpleDirectoryReader('data').load_data()

# 2. 解析节点
parser = SimpleNodeParser.from_defaults()
nodes = parser.get_nodes_from_documents(documents)

# 3. 创建索引
index = VectorStoreIndex(nodes)

# 4. 查询引擎
query_engine = index.as_query_engine(
    llm=OpenAI(model="gpt-4"),
    similarity_top_k=5
)

# 5. 查询
response = query_engine.query("Agent的工作原理是什么?")
print(response)
```

#### LlamaIndex vs LangChain

```
LangChain:
✅ Agent能力更强
✅ 工具生态丰富
✅ 社区活跃
⚠️ 复杂度高

LlamaIndex:
✅ RAG能力强
✅ 数据处理优秀
✅ API简洁
⚠️ Agent功能相对弱

建议: 两者结合使用!
```

---

### 3. AutoGen - 多Agent协作 ⭐⭐⭐⭐

#### 简介
```
开发者: Microsoft
定位: 多Agent协作框架
特点: Agent对话、自动协作
适合: 复杂任务、团队协作
```

#### 示例:双Agent协作

```python
import autogen

# 1. 配置
config_list = [{
    "model": "gpt-4",
    "api_key": "your-key"
}]

# 2. 创建助手Agent
assistant = autogen.AssistantAgent(
    name="assistant",
    llm_config={
        "config_list": config_list,
        "temperature": 0
    }
)

# 3. 创建用户代理
user_proxy = autogen.UserProxyAgent(
    name="user_proxy",
    human_input_mode="NEVER",
    max_consecutive_auto_reply=10,
    code_execution_config={
        "work_dir": "coding",
        "use_docker": False
    }
)

# 4. 开始对话
user_proxy.initiate_chat(
    assistant,
    message="写一个Python爬虫爬取新闻"
)
```

---

### 4. CrewAI - 角色扮演 ⭐⭐⭐

#### 简介
```
定位: 角色化Agent团队
特点: 明确分工、协作流程
适合: 模拟团队工作流
```

#### 示例:研究团队

```python
from crewai import Agent, Task, Crew

# 1. 定义研究员
researcher = Agent(
    role='研究员',
    goal='研究AI领域最新进展',
    backstory='你是AI领域的专家研究员',
    verbose=True
)

# 2. 定义作者
writer = Agent(
    role='技术作者',
    goal='将研究成果写成文章',
    backstory='你是经验丰富的技术作家',
    verbose=True
)

# 3. 定义任务
research_task = Task(
    description='研究2024年Agent技术进展',
    agent=researcher
)

write_task = Task(
    description='将研究成果写成博客文章',
    agent=writer
)

# 4. 组建团队
crew = Crew(
    agents=[researcher, writer],
    tasks=[research_task, write_task],
    verbose=True
)

# 5. 执行
result = crew.kickoff()
```

---

## 第二部分:MCP协议深入

### MCP (Model Context Protocol)

#### 什么是MCP?

```
定义:
- Anthropic开发的标准协议
- 连接AI模型和外部工具
- 类似于"Agent的USB接口"

作用:
✅ 标准化工具调用
✅ 上下文管理
✅ 多模型兼容
✅ 安全隔离
```

#### MCP架构

```
┌─────────────┐
│   Client    │ (Agent应用)
│  (Your App) │
└──────┬──────┘
       │ MCP Protocol
       ↓
┌─────────────┐
│  MCP Server │ (工具提供者)
│   (Tools)   │
└──────┬──────┘
       │
       ↓
┌─────────────┐
│  Resources  │ (文件、API、数据库等)
└─────────────┘
```

#### MCP Server示例

```python
from mcp.server import Server
from mcp.types import Tool, TextContent

# 1. 创建MCP Server
server = Server("my-tools")

# 2. 定义工具
@server.tool()
async def search_code(
    query: str,
    language: str = "python"
) -> str:
    """搜索代码片段"""
    # 实现搜索逻辑
    return f"搜索 {language} 代码: {query}"

@server.tool()
async def run_code(
    code: str,
    language: str = "python"
) -> str:
    """执行代码"""
    # 实现代码执行
    return f"执行结果: ..."

# 3. 定义资源
@server.resource("file://{path}")
async def read_file(path: str) -> str:
    """读取文件"""
    with open(path) as f:
        return f.read()

# 4. 启动服务
if __name__ == "__main__":
    server.run()
```

#### MCP Client使用

```python
from mcp.client import Client

# 1. 连接MCP Server
client = Client()
await client.connect("my-tools")

# 2. 列出可用工具
tools = await client.list_tools()
print(tools)

# 3. 调用工具
result = await client.call_tool(
    "search_code",
    arguments={"query": "快速排序", "language": "python"}
)
print(result)

# 4. 访问资源
content = await client.read_resource("file:///path/to/file.txt")
print(content)
```

---

## 第三部分:CLI工具开发

### 您已了解的库深入应用

#### 1. Rich - 美化输出 ⭐⭐⭐⭐⭐

```python
from rich.console import Console
from rich.progress import track
from rich.table import Table
from rich.panel import Panel
from rich.markdown import Markdown
from rich.syntax import Syntax

console = Console()

# Agent对话界面
def agent_chat():
    # 欢迎面板
    console.print(Panel(
        "[bold blue]AI Agent[/bold blue]\n"
        "你的智能助手",
        title="欢迎",
        border_style="blue"
    ))

    while True:
        # 用户输入
        user_input = console.input("[bold green]You:[/bold green] ")

        # 显示思考过程
        with console.status("[bold yellow]思考中...", spinner="dots"):
            response = agent.run(user_input)

        # Agent回复
        console.print(Panel(
            response,
            title="[bold magenta]Agent",
            border_style="magenta"
        ))

# 显示Agent工具调用
def show_tool_calls(tools):
    table = Table(title="工具调用记录")
    table.add_column("工具", style="cyan")
    table.add_column("参数", style="green")
    table.add_column("结果", style="yellow")

    for tool in tools:
        table.add_row(
            tool.name,
            str(tool.args),
            tool.result[:50] + "..."
        )

    console.print(table)

# 显示代码
def show_code(code, language="python"):
    syntax = Syntax(code, language, theme="monokai")
    console.print(Panel(syntax, title="生成的代码"))
```

#### 2. PromptToolkit - 交互式输入 ⭐⭐⭐⭐⭐

```python
from prompt_toolkit import PromptSession
from prompt_toolkit.completion import WordCompleter
from prompt_toolkit.history import FileHistory
from prompt_toolkit.auto_suggest import AutoSuggestFromHistory
from prompt_toolkit.styles import Style

# 1. 自定义样式
style = Style.from_dict({
    'prompt': 'ansicyan bold',
    'input': 'ansigreen',
})

# 2. 命令补全
commands = WordCompleter([
    '/help', '/clear', '/exit',
    '/tools', '/history', '/save'
], ignore_case=True)

# 3. 创建会话
session = PromptSession(
    history=FileHistory('.agent_history'),
    auto_suggest=AutoSuggestFromHistory(),
    completer=commands,
    style=style
)

# 4. Agent交互循环
def agent_repl():
    console.print("[bold]Agent CLI v1.0")
    console.print("输入 /help 查看帮助\n")

    while True:
        try:
            # 获取用户输入
            user_input = session.prompt(
                '>>> ',
                style=style
            )

            # 处理命令
            if user_input.startswith('/'):
                handle_command(user_input)
            else:
                # Agent处理
                response = agent.run(user_input)
                console.print(f"\n{response}\n")

        except KeyboardInterrupt:
            continue
        except EOFError:
            break

def handle_command(cmd):
    if cmd == '/help':
        show_help()
    elif cmd == '/tools':
        list_tools()
    elif cmd == '/history':
        show_history()
    elif cmd == '/exit':
        console.print("再见!")
        exit(0)
```

#### 3. Click/Typer - 命令行框架 ⭐⭐⭐⭐

##### Typer示例(更现代)

```python
import typer
from typing import Optional
from enum import Enum

app = typer.Typer()

class Model(str, Enum):
    gpt4 = "gpt-4"
    gpt35 = "gpt-3.5-turbo"
    claude = "claude-3"

@app.command()
def chat(
    message: str = typer.Argument(..., help="要发送的消息"),
    model: Model = typer.Option(Model.gpt4, help="使用的模型"),
    temperature: float = typer.Option(0.7, help="温度参数"),
    verbose: bool = typer.Option(False, "--verbose", "-v", help="详细输出")
):
    """
    与Agent对话
    """
    if verbose:
        typer.echo(f"使用模型: {model.value}")
        typer.echo(f"温度: {temperature}")

    # Agent处理
    response = agent.run(message, model=model.value, temperature=temperature)
    typer.echo(response)

@app.command()
def tools():
    """列出可用工具"""
    tool_list = agent.list_tools()

    for tool in tool_list:
        typer.echo(f"- {tool.name}: {tool.description}")

@app.command()
def interactive():
    """交互式模式"""
    agent_repl()  # 使用上面的PromptToolkit

if __name__ == "__main__":
    app()
```

使用:
```bash
# 单次对话
python agent.py chat "帮我搜索Python教程"

# 指定模型
python agent.py chat "写个排序算法" --model gpt-3.5-turbo

# 详细输出
python agent.py chat "分析这段代码" -v

# 列出工具
python agent.py tools

# 交互模式
python agent.py interactive
```

---

## 第四部分:完整Agent应用架构

### 项目结构

```
agent-project/
├── src/
│   ├── agent/
│   │   ├── __init__.py
│   │   ├── core.py           # Agent核心逻辑
│   │   ├── tools.py          # 工具定义
│   │   ├── memory.py         # 记忆管理
│   │   └── prompts.py        # Prompt模板
│   │
│   ├── mcp/
│   │   ├── __init__.py
│   │   ├── server.py         # MCP Server
│   │   └── client.py         # MCP Client
│   │
│   ├── cli/
│   │   ├── __init__.py
│   │   ├── main.py           # CLI入口
│   │   ├── commands.py       # 命令处理
│   │   └── ui.py             # UI组件
│   │
│   ├── web/
│   │   ├── __init__.py
│   │   ├── app.py            # Flask应用
│   │   ├── routes.py         # 路由
│   │   └── templates/        # 模板
│   │
│   ├── vector/
│   │   ├── __init__.py
│   │   ├── store.py          # 向量存储
│   │   └── retriever.py      # 检索器
│   │
│   └── utils/
│       ├── __init__.py
│       ├── config.py         # 配置
│       └── logger.py         # 日志
│
├── tests/
│   ├── test_agent.py
│   ├── test_tools.py
│   └── test_mcp.py
│
├── data/                     # 知识库
├── logs/                     # 日志
├── pyproject.toml           # 依赖管理
├── Dockerfile
└── README.md
```

### 核心代码示例

#### agent/core.py
```python
from langchain.agents import AgentExecutor, create_openai_tools_agent
from langchain_openai import ChatOpenAI
from langchain.memory import ConversationBufferMemory
from .tools import get_tools
from .prompts import get_agent_prompt

class Agent:
    def __init__(self, model="gpt-4", temperature=0.7):
        self.llm = ChatOpenAI(model=model, temperature=temperature)
        self.tools = get_tools()
        self.memory = ConversationBufferMemory(
            memory_key="chat_history",
            return_messages=True
        )

        # 创建Agent
        prompt = get_agent_prompt()
        agent = create_openai_tools_agent(
            self.llm,
            self.tools,
            prompt
        )

        # 执行器
        self.executor = AgentExecutor(
            agent=agent,
            tools=self.tools,
            memory=self.memory,
            verbose=True,
            handle_parsing_errors=True
        )

    def run(self, input_text: str) -> str:
        """运行Agent"""
        try:
            result = self.executor.invoke({"input": input_text})
            return result["output"]
        except Exception as e:
            return f"错误: {str(e)}"

    def reset_memory(self):
        """重置记忆"""
        self.memory.clear()
```

#### cli/main.py
```python
import typer
from rich.console import Console
from ..agent.core import Agent
from .ui import agent_repl

app = typer.Typer()
console = Console()
agent = None

@app.callback()
def init(
    model: str = typer.Option("gpt-4", help="模型名称"),
    temperature: float = typer.Option(0.7, help="温度")
):
    """初始化Agent"""
    global agent
    agent = Agent(model=model, temperature=temperature)

@app.command()
def chat(message: str):
    """单次对话"""
    response = agent.run(message)
    console.print(response)

@app.command()
def interactive():
    """交互式模式"""
    agent_repl(agent, console)

@app.command()
def web(port: int = 5000):
    """启动Web界面"""
    from ..web.app import create_app
    app = create_app(agent)
    app.run(port=port)

if __name__ == "__main__":
    app()
```

---

## 第五部分:Rust优化Python Agent

### 性能瓶颈

```
Python Agent的性能瓶颈:
├── 文本处理(分词、匹配)
├── 向量计算(Embedding相似度)
├── 数据解析(JSON/XML)
└── 并发处理(GIL限制)

Rust可以优化的部分:
✅ 文本处理引擎
✅ 向量相似度计算
✅ 数据序列化/反序列化
✅ 并发任务调度
```

### PyO3: Python调用Rust

#### 1. Rust端实现

```rust
// Cargo.toml
[dependencies]
pyo3 = { version = "0.20", features = ["extension-module"] }
numpy = "0.20"

// src/lib.rs
use pyo3::prelude::*;
use pyo3::types::PyList;

/// 文本相似度计算(余弦相似度)
#[pyfunction]
fn cosine_similarity(vec1: Vec<f32>, vec2: Vec<f32>) -> PyResult<f32> {
    if vec1.len() != vec2.len() {
        return Err(PyErr::new::<pyo3::exceptions::PyValueError, _>(
            "向量长度不匹配"
        ));
    }

    let dot_product: f32 = vec1.iter()
        .zip(vec2.iter())
        .map(|(a, b)| a * b)
        .sum();

    let norm1: f32 = vec1.iter().map(|x| x * x).sum::<f32>().sqrt();
    let norm2: f32 = vec2.iter().map(|x| x * x).sum::<f32>().sqrt();

    Ok(dot_product / (norm1 * norm2))
}

/// 批量相似度计算
#[pyfunction]
fn batch_similarity(
    query: Vec<f32>,
    documents: Vec<Vec<f32>>
) -> PyResult<Vec<f32>> {
    documents.iter()
        .map(|doc| cosine_similarity(query.clone(), doc.clone()))
        .collect()
}

/// 文本分词(简单示例)
#[pyfunction]
fn tokenize(text: &str) -> PyResult<Vec<String>> {
    Ok(text.split_whitespace()
        .map(|s| s.to_string())
        .collect())
}

#[pymodule]
fn rust_agent(_py: Python, m: &PyModule) -> PyResult<()> {
    m.add_function(wrap_pyfunction!(cosine_similarity, m)?)?;
    m.add_function(wrap_pyfunction!(batch_similarity, m)?)?;
    m.add_function(wrap_pyfunction!(tokenize, m)?)?;
    Ok(())
}
```

#### 2. Python端使用

```python
# 编译: maturin develop
import rust_agent
import numpy as np

# 1. 文本分词
text = "Agent 是智能的助手"
tokens = rust_agent.tokenize(text)
print(tokens)  # ['Agent', '是', '智能的', '助手']

# 2. 相似度计算
vec1 = [0.1, 0.2, 0.3, 0.4]
vec2 = [0.2, 0.3, 0.4, 0.5]
similarity = rust_agent.cosine_similarity(vec1, vec2)
print(f"相似度: {similarity}")

# 3. 批量计算
query = [0.1, 0.2, 0.3]
docs = [
    [0.2, 0.3, 0.4],
    [0.1, 0.1, 0.1],
    [0.3, 0.3, 0.3]
]
similarities = rust_agent.batch_similarity(query, docs)
print(f"相似度列表: {similarities}")

# 性能对比
import time

# Python版本
def cosine_similarity_py(v1, v2):
    import numpy as np
    return np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))

# 测试
v1 = np.random.rand(1536).tolist()  # OpenAI Embedding维度
v2 = np.random.rand(1536).tolist()

# Python
start = time.time()
for _ in range(10000):
    cosine_similarity_py(v1, v2)
py_time = time.time() - start

# Rust
start = time.time()
for _ in range(10000):
    rust_agent.cosine_similarity(v1, v2)
rust_time = time.time() - start

print(f"Python: {py_time:.4f}s")
print(f"Rust: {rust_time:.4f}s")
print(f"加速比: {py_time/rust_time:.2f}x")
```

---

## 第六部分:实战项目建议

### 项目1: 智能代码助手 (推荐!)

```
功能:
├── 代码搜索
│   └── 向量检索+语义搜索
├── 代码解释
│   └── LLM分析代码
├── 代码生成
│   └── 根据需求生成代码
├── Bug修复
│   └── 自动检测并修复
└── 文档生成
    └── 自动生成文档

技术栈:
Python: LangChain + LlamaIndex
CLI: Rich + PromptToolkit + Typer
MCP: 工具标准化
Rust: 代码解析(可选)
```

### 项目2: 知识库问答系统

```
功能:
├── 文档上传
├── 智能索引
├── 语义搜索
├── 多轮对话
└── 引用溯源

技术栈:
Python: LlamaIndex (RAG)
向量库: Chroma/Pinecone
Web: Flask
CLI: Typer + Rich
```

### 项目3: 多Agent协作系统

```
功能:
├── 研究Agent (搜索资料)
├── 分析Agent (分析数据)
├── 写作Agent (生成报告)
└── 评审Agent (质量检查)

技术栈:
Python: AutoGen/CrewAI
MCP: Agent间通信
Web: Flask展示
```

---

## 学习路线图

### Month 1: 基础掌握
```
Week 1: LangChain基础
├── 安装配置
├── LLM调用
├── Prompt工程
└── 简单链

Week 2: Agent开发
├── ReAct Agent
├── 工具定义
├── Function Calling
└── 记忆管理

Week 3: RAG系统
├── 文档加载
├── 向量存储
├── 检索优化
└── QA链

Week 4: CLI开发
├── Rich界面
├── PromptToolkit交互
├── Typer命令
└── 整合Agent
```

### Month 2-3: 进阶项目
```
实现完整Agent应用
├── MCP协议集成
├── 多工具支持
├── Web界面
├── 部署上线
└── 开源发布
```

---

## 最后的建议

### 您的优势

```
✅ Java后端 - 稳定的API服务
✅ Python AI - 核心Agent逻辑
✅ Flask - 快速Web原型
✅ CLI工具 - 用户体验
✅ Linux - 开发环境
✅ Rust - 性能优化(可选)

这是完美的Agent开发栈!
```

### 立即开始

```
本周:
[ ] 安装LangChain
[ ] 实现第一个Agent
[ ] 添加Rich输出
[ ] GitHub开源

本月:
[ ] 完成代码助手项目
[ ] 集成MCP协议
[ ] 写技术博客
[ ] 持续优化
```

**Agent开发是未来,您已经准备好了!** 🚀
