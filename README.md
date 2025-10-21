# C++ 高性能 Web 服务示例 (C++ Core + Python API + Vue Frontend)

本项目演示了如何将高性能的 C++ 算法集成到现代 Web 架构中，利用 C++ 的计算速度优势，并通过 Python FastAPI 封装为 RESTful API，最终由 Vue.js 前端进行调用和展示。

**项目核心功能：** 前端输入数字 N，后端调用 C++ 核心计算斐波那契数列的第 N 位。

## ⚙️ 架构概览

| 组件 | 端口 | 技术栈 | 职责 |
| :--- | :--- | :--- | :--- |
| **C++ Core** | N/A | C++17, Pybind11 | 核心高性能计算（libcore.so 动态库） |
| **Backend API** | `8000` | Python (FastAPI), Uvicorn | 接收 HTTP 请求，通过 Pybind11 调用 C++ 核心，并返回 JSON 结果。 |
| **Frontend UI** | `5173` | Vue 3, Vite, Axios | 用户界面展示，向后端 API (8000 端口) 发起请求并渲染结果。 |

## 🚀 本地开发环境设置 (Kali Linux)

本项目强烈推荐在 Linux 环境（如 Kali）下，使用 Python **虚拟环境**进行开发。

### 1. 环境准备

确保您的系统安装了以下依赖：

* `git`
* `python3`, `python3-pip`
* `g++` (C++ 编译器)
* `python3-dev` (包含 C++ 编译所需的 `Python.h` 头文件)
* `node` 和 `npm` (用于前端)

```bash
# 安装 C++ 编译所需包和 Python venv 工具
sudo apt update
sudo apt install build-essential python3-venv python3-dev

2. 项目初始化
在项目根目录 (~/cpp-web-service/) 下创建并激活虚拟环境，并安装 Python 依赖。

Bash

# 1. 进入项目根目录
cd ~/cpp-web-service

# 2. 创建虚拟环境并激活 (重要!)
python3 -m venv .venv
source .venv/bin/activate 

# 3. 安装 Python 依赖 (在激活的虚拟环境中)
pip install fastapi uvicorn python-multipart pybind11
3. C++ 核心编译
将 cpp_core/calculator.cpp 编译为 backend/libcore.so。编译时需指定 Python 和 Pybind11 的头文件路径。

Bash

# 确保虚拟环境已激活，且位于项目根目录

# 1. 自动获取编译路径
PYBIND11_INCLUDE=$(python3 -c "import pybind11; print(pybind11.get_include())")
PYTHON_HEADER_DIR="/usr/include/python3.13" # **请根据实际情况修改此路径**
PYTHON_LDFLAGS=$(python3-config --ldflags)

# 2. 执行编译命令 (使用手动指定的头文件路径和链接库)
g++ -O3 -Wall -shared -std=c++17 -fPIC \
    -I$PYTHON_HEADER_DIR \
    -I$PYBIND11_INCLUDE \
    cpp_core/calculator.cpp \
    -o backend/libcore.so \
    -lpython3.13 # 手动链接 Python 库，确保成功
成功标志： backend/ 目录下生成 libcore.so 文件。

4. 启动后端 API 服务
在 backend/ 目录下启动 FastAPI 服务，它会加载刚刚编译好的 libcore.so。

Bash

cd backend
# 确保虚拟环境已激活
uvicorn main:app --reload --host 0.0.0.0 --port 8000
服务地址： http://localhost:8000/。请让这个终端窗口保持开启。

5. 启动前端 UI 服务
在另一个终端窗口中，进入 frontend/ 目录，安装 Node.js 依赖并启动 Vue/Vite 开发服务器。

Bash

# 切换到前端目录
cd ~/cpp-web-service/frontend

# 1. 检查和安装 Node.js 依赖 (如果 package.json 丢失，请重新 vite 初始化)
npm install 
npm install axios

# 2. 运行前端
npm run dev
访问地址： 浏览器打开前端提供的 URL (通常是 http://localhost:5173/)。

💻 Git 提交与远程推送
如果您尚未将项目推送到远程仓库，请按照以下步骤操作：

Bash

# 确保在项目根目录
cd ~/cpp-web-service

# 1. 初始化 Git 仓库
git init

# 2. 关联远程仓库 (替换 YOUR_REMOTE_URL)
git remote add origin <YOUR_REMOTE_URL>

# 3. 添加所有文件，包括 README.md
git add .

# 4. 提交
git commit -m "feat: Add C++ core, FastAPI backend, and complete documentation in README"

# 5. 推送至远程仓库
git push -u origin main