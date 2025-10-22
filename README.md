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


## 🔒 本地仿真部署与持久化运行

为了模拟生产环境和测试前后端在不同接口上的通信，我们在虚拟机上使用 Gunicorn/Nohup 进行了持久化部署。

### 6. 配置防火墙 (允许外部访问)

为了允许宿主机（物理机）通过 IP 地址访问虚拟机内部的 5173 (前端) 和 8000 (后端) 端口，需要配置 Kali 虚拟机的防火墙。

```bash
# 检查 ufw 是否安装
sudo apt install ufw 

# 允许 5173 和 8000 端口
sudo ufw allow 5173/tcp
sudo ufw allow 8000/tcp

# 开启防火墙 (如果未开启)
sudo ufw enable
7. 持久化运行后端 API (生产模式)
在本地测试环境中，我们使用 Gunicorn 和 nohup 来让后端服务在终端关闭后仍持续运行。

Bash

cd backend

# 安装 Gunicorn (如果之前没有安装)
pip install gunicorn

# 使用 nohup 和 Gunicorn 启动服务，并绑定到所有外部接口 (0.0.0.0)
nohup gunicorn main:app --workers 4 --worker-class uvicorn.workers.UvicornWorker --bind 0.0.0.0:8000 &
停止服务： 如果需要停止服务，请使用 ps aux | grep gunicorn 查找进程 PID，然后使用 kill <PID> 命令终止它。

8. 宿主机访问配置（Vite Host 绑定）
为了让宿主机（物理机）能通过虚拟机的 IP 访问前端服务，需要配置 Vite 监听所有网络接口。

修改 frontend/vite.config.ts：

在 export default defineConfig(...) 中添加或修改 server 配置：

TypeScript

// ... 其他配置 ...
  server: {
    host: '0.0.0.0', // 允许通过 IP 地址访问
    port: 5173,      
  }
// ...
9. 访问与验证
完成以上所有步骤后：

查找虚拟机 IP： 在 Kali 终端运行 ip a，找到你的虚拟机 IP (例如 192.168.x.x)。

访问： 在宿主机浏览器中，通过 IP 访问前端：

http://[你的虚拟机IP]:5173/

验证： 点击前端按钮，检查后端 Gunicorn 的 nohup.out 日志和浏览器网络请求，确认请求能够成功从 5173 发送到 8000 端口并获得 C++ 计算结果。

步骤 2: 提交更改到远程仓库
现在，将更新后的 README.md 文件提交到远程 Git 仓库：

Bash

# 确保在项目根目录
cd ~/cpp-web-service

# 1. 添加更新后的 README 文件
git add README.md
git add frontend/vite.config.ts # 如果你修改了 vite.config.ts 也应该添加

# 2. 提交更改
git commit -m "docs: Add local emulation deployment and persistence steps to README"

# 3. 推送至远程仓库 (使用你之前设置的正确分支名，例如 main)
git push


# 🚀 AccelCompress：C++ 驱动的高性能媒体优化 API

本项目是一个以 C++ 高性能计算为核心的商业化 Web 服务蓝图，旨在利用 C/C++ 对计算资源和内存的精细控制，提供比传统 Python/Node.js 服务更快的图片压缩和优化 API。

**核心价值：速度就是金钱。我们提供市场领先的计算性能。**

## ⚙️ 架构与技术栈

| 组件 | 角色 | 技术栈 | 性能优势来源 |
| :--- | :--- | :--- | :--- |
| **C++ Core** | 核心计算引擎 | C++17, Pybind11, libjpeg-turbo/OpenCV | **内存和线程控制**，实现极速图片处理。 |
| **Backend API** | 服务接口层 | Python (FastAPI), Gunicorn | **高并发异步处理**，封装 C++ 核心，提供稳定的 RESTful API。 |
| **Frontend UI/Docs** | 营销与文档 | Vue 3, Vite, Axios | 客户门户、API 文档展示、支付/Key 管理界面。 |

## 💡 商业计划：按量计费服务

本项目将通过提供高性能的按量计费 API 服务实现被动收入，目标客户是需要快速加载速度的电商、媒体和应用开发者。

### 变现模式：月度订阅与按需付费

| 计划名称 | 月费 | 包含的 API 调用次数 | 溢出费率 | 适合用户 |
| :--- | :--- | :--- | :--- | :--- |
| **Free Tier (免费体验)** | $0 | 500 次/月 | N/A | 个人开发者、博客测试 |
| **Developer (开发者)** | $10 | 10,000 次/月 | $1 / 1000 次 | 小型网站、初创企业 |
| **Business (企业)** | $50 | 100,000 次/月 | $0.5 / 1000 次 | 电商平台、高流量媒体 |

### 核心竞争优势

1.  **极速性能：** 核心压缩/优化算法全部在 C++ 中运行，CPU 密集型任务的性能领先市场同类产品 **3-5 倍**。
2.  **WebP 专家：** 专注于高效的 WebP (无损/有损) 转换和优化，这是现代 Web 性能优化的标准。
3.  **高并发稳定：** 利用 FastAPI 的异步特性和 C++ 的高效执行，确保 API 在高并发请求下依然稳定可靠。

## 🎯 行动计划 (您需要实施的步骤)

| 阶段 | 任务 | 目标 |
| :--- | :--- | :--- |
| **MVP 优化** | **升级 C++ 核心** | 将斐波那契计算替换为真正的图片优化算法（例如，使用 libjpeg-turbo 优化 JPEG 渐进式渲染）。 |
| **后端强化** | **实现 API Key 认证** | 在 FastAPI 中搭建 JWT 或简单的 Key 验证体系，确保只有付费用户才能调用服务。 |
| **前端开发** | **支付与 Key 生成界面** | 在 Vue 中创建用户注册、API Key 展示和**集成 Stripe/PayPal** 的支付流程。 |
| **部署上线** | **云服务器部署** | 将 C++ 核心、Gunicorn/FastAPI 部署到云服务器（如 AWS/DigitalOcean），确保 24/7 运行。 |

---

## 🛠️ 本地开发与运行指南 (C++ Core + Python API)

### 1. 环境准备

```bash
# 安装必要工具 (Kali Linux)
sudo apt install build-essential python3-venv python3-dev git nodejs npm

# 初始化并激活虚拟环境
python3 -m venv .venv
source .venv/bin/activate 

# 安装 Python 依赖
pip install fastapi uvicorn python-multipart pybind11 gunicorn