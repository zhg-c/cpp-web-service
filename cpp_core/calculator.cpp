#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

// 修复：引入 setjmp.h 用于错误处理
#include <setjmp.h> 

// 引入 libjpeg-turbo 的头文件
#include <stdio.h>
#include <jpeglib.h>

namespace py = pybind11;

// --- libjpeg 错误处理结构 ---
struct jpeg_error_mgr_pybind {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};

// 错误退出函数
// 使用 METHODDEF 宏 (在 jpeglib.h 中定义)
METHODDEF(void) jpeg_error_exit(j_common_ptr cinfo) {
    jpeg_error_mgr_pybind *err = (jpeg_error_mgr_pybind *)cinfo->err;
    char buffer[JMSG_LENGTH_MAX];
    (*cinfo->err->format_message) (cinfo, buffer);
    
    // 抛出 C++ 异常，同时执行 longjmp
    throw std::runtime_error("libjpeg Error: " + std::string(buffer));
    longjmp(err->setjmp_buffer, 1);
}

// --- 核心优化函数 ---
/**
 * @brief 使用 libjpeg-turbo 进行图片优化
 * @param image_bytes 原始 JPEG 图片文件的字节数据
 * @param quality 目标 JPEG 质量 (1-100)
 * @return std::vector<unsigned char> 优化后的图片字节数据
 */
std::vector<unsigned char> optimize_jpeg(
    const std::vector<unsigned char>& image_bytes, 
    int quality = 80
) {
    if (quality < 1 || quality > 100) {
        quality = 80;
    }
    
    // --- 1. JPEG 解码器设置 ---
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr_pybind jerr;
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = jpeg_error_exit;
    
    // 设置错误捕捉点
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_decompress(&cinfo);
        throw std::runtime_error("JPEG decoding failed at setjmp.");
    }
    
    jpeg_create_decompress(&cinfo);
    
    // 设置内存作为输入源
    jpeg_mem_src(&cinfo, image_bytes.data(), image_bytes.size());

    // 读取文件头部信息
    if (jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK) {
        jpeg_destroy_decompress(&cinfo);
        throw std::runtime_error("JPEG header read failed or not a JPEG file.");
    }

    // 设置解码参数
    cinfo.out_color_space = JCS_RGB; 
    jpeg_start_decompress(&cinfo);

    int row_stride = cinfo.output_width * cinfo.output_components;
    long total_size = (long)cinfo.output_width * cinfo.output_height * cinfo.output_components;
    
    std::vector<unsigned char> raw_image_data(total_size);
    unsigned char *buffer_ptr = raw_image_data.data();
    
    // 读取图像数据
    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned char *row_pointer[1];
        row_pointer[0] = buffer_ptr + cinfo.output_scanline * row_stride;
        jpeg_read_scanlines(&cinfo, row_pointer, 1);
    }
    
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    // --- 2. JPEG 编码器设置 (压缩) ---
    struct jpeg_compress_struct cinfo_out;
    cinfo_out.err = jpeg_std_error(&jerr.pub);
    
    if (setjmp(jerr.setjmp_buffer)) {
        jpeg_destroy_compress(&cinfo_out);
        throw std::runtime_error("JPEG encoding failed at setjmp.");
    }

    jpeg_create_compress(&cinfo_out);
    
    // 设置输出目标为内存缓冲区
    unsigned char *outbuffer = nullptr;
    unsigned long outsize = 0;
    jpeg_mem_dest(&cinfo_out, &outbuffer, &outsize);

    // 配置编码参数
    cinfo_out.image_width = cinfo.output_width;
    cinfo_out.image_height = cinfo.output_height;
    cinfo_out.input_components = cinfo.output_components; 
    cinfo_out.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo_out);
    jpeg_set_quality(&cinfo_out, quality, TRUE); // 设置目标压缩质量
    
    // 开始压缩
    jpeg_start_compress(&cinfo_out, TRUE);

    // 写入图像数据
    JSAMPROW row_pointer;
    for (JDIMENSION i = 0; i < cinfo_out.image_height; i++) {
        row_pointer = raw_image_data.data() + i * row_stride;
        jpeg_write_scanlines(&cinfo_out, &row_pointer, 1);
    }

    // 完成压缩
    jpeg_finish_compress(&cinfo_out);

    // --- 3. 返回结果并清理 ---
    std::vector<unsigned char> compressed_data(outbuffer, outbuffer + outsize);
    
    // 释放 libjpeg 分配的内存
    if (outbuffer) {
        free(outbuffer);
    }
    
    jpeg_destroy_compress(&cinfo_out);

    return compressed_data;
}

// --- Pybind11 模块定义 ---
// cpp_core/calculator.cpp

PYBIND11_MODULE(libcore, m) {
    m.doc() = "AccelCompress C++ core for high-performance JPEG optimization."; 

    // ✨ 最终修复：使用 py::bytes 接收输入，消除输入参数的类型歧义
    m.def("optimize_jpeg", [](py::bytes image_bytes_py, int quality) {
        
        // 1. 将 Python bytes 显式转换为 C++ std::vector<unsigned char>
        //    通过 py::bytes 的方法获取数据的原始指针和大小
        std::string buffer = image_bytes_py.cast<std::string>();
        std::vector<unsigned char> image_bytes_cpp(buffer.begin(), buffer.end());

        // 2. 调用 C++ 实际函数
        // 注意：这里需要调用你原来在 calculator.cpp 中定义的 C++ 函数
        std::vector<unsigned char> result = optimize_jpeg(image_bytes_cpp, quality);
        
        // 3. 显式将 C++ 字节向量转换为 Python bytes
        return py::bytes((const char*)result.data(), result.size());
    },
    "Optimizes a JPEG file's bytes by re-encoding with a target quality.",
    py::arg("image_bytes"),
    py::arg("quality") = 80);
}