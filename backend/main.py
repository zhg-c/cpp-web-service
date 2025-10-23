# backend/main.py

from fastapi import FastAPI, File, UploadFile, HTTPException, Form 
from fastapi.middleware.cors import CORSMiddleware
from typing import Optional

# 修复：导入 Response 类
from starlette.responses import Response 

# --- 导入 C++ 核心 ---
try:
    import libcore
except ImportError as e:
    print(f"FATAL ERROR: Failed to import C++ core (libcore.so). Details: {e}")
    pass


# --- FastAPI 实例 ---
app = FastAPI(
    title="AccelCompress C++ High-Performance API",
    description="API for high-speed image optimization powered by C++ core."
)

# --- CORS 配置 ---
origins = [
    "http://localhost",
    "http://localhost:5173", 
    "http://127.0.0.1:5173",
    "*" 
]

app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


# --- C++ 驱动的图片处理 API 路由 ---
@app.post("/api/optimize")
async def optimize_image(
    image_file: UploadFile = File(...),
    quality: int = Form(80), # 接收质量参数
    api_key: Optional[str] = Form(None)
):
    """
    接收图片文件，通过 Python 调用 C++ 核心函数进行 JPEG 优化。
    """
    
    # 仅允许 JPEG 文件以确保 C++ 逻辑正确
    if not image_file.content_type or 'image/jpeg' not in image_file.content_type:
        raise HTTPException(status_code=400, detail=f"Unsupported media type: {image_file.content_type}. Only JPEG files are supported by the C++ core in this MVP.")

    try:
        original_bytes = await image_file.read()
        original_size = len(original_bytes)
        
        # 2. 核心步骤：调用 C++ 优化函数
        if 'libcore' not in globals():
             raise HTTPException(status_code=500, detail="C++ Core (libcore) module not loaded. Check installation/path.")

        # 调用新的优化函数
        optimized_bytes = libcore.optimize_jpeg(original_bytes, quality)
        optimized_size = len(optimized_bytes)
        
        # 3. 返回优化后的图片文件
        return Response(
            content=optimized_bytes, 
            media_type="image/jpeg",
            headers={
                # 添加自定义头信息，方便前端展示
                "X-Original-Size": str(original_size),
                "X-Optimized-Size": str(optimized_size),
                "X-Compression-Ratio": f"{((original_size - optimized_size) / original_size * 100):.2f}%"
            }
        )

    except ImportError as e:
        raise HTTPException(status_code=500, detail=f"C++ Core import failed: {e}")
    except Exception as e:
        # 捕获 C++ 内部抛出的 std::runtime_error 和其他异常
        raise HTTPException(status_code=500, detail=f"Processing failed: {e}")


# --- 根路由 (健康检查) ---
@app.get("/")
def read_root():
    return {"message": "AccelCompress Backend API is running."}