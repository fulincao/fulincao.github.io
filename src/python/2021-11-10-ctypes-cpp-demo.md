---
title: cython调用c++库
date: 2021-11-10
categories: [python编程]
tags: [cython, ctypes]     # TAG names should always be lowercase
---
# cython调用c++动态库示例

- cpp代码暴露的函数如下

```cpp

typedef struct can_frame{
    int can_id;
    uint8_t can_data[8];
}can_frame;

typedef struct singal
{
    int start;
    int length;
    float scale;
    float offest;
    float min_val;
    float max_val;
    bool little_order;
    bool is_unsigned;
    char* name;
}singal;

extern "C" {
    float decode(singal s, can_frame t);
    void add_dbc(char* dbc_path);
    char* decode_message(can_frame t, bool use_val_table=true);
}

```

- 在cmake中通过 **add_library(test_can SHARED can_parser.cpp)** 导出libtest_can.so,至此动态库导出完成。接下来python调用

```python
## 导入ctypes
from ctypes import *
## 加载动态库
decode_dll = cdll.LoadLibrary("/home/cao/CLionProjects/pcv/build/lib/libtest_can.so")

## 定义结构体，同cpp定义一致,左侧为变量名，右侧为类型。其中c语言与ctypes的类型转换见末尾
class singal(Structure):
    _fields_ = [
        ("start", c_int),
        ("length", c_int),
        ("scale", c_float),
        ("offest", c_float),
        ("min_val", c_float),
        ("max_val", c_float),
        ("little_order", c_bool),
        ("is_unsigned", c_bool),
        ("name", c_char_p)
    ]

class can_frame(Structure):
    _fields_ = [
        ("can_id", c_int),
        ("can_data", c_ubyte*8)
    ]

## 调用add_dbc函数
name = c_char_p(bytes("./dbc/QZ_x1q_20200224.dbc", "utf-8")) # str -> bytes -> c_char_p
decode_dll.add_dbc(name)

## 调用decode
can_id = 10086
can_data = [0, 1, 2, 3, 4, 5, 6, 7]
data = (c_ubyte*8)(*can_data)
c = can_frame(can_id, data)
s = singal(8, 12, 0.0625, 0, 0, 250, True, True, c_char_p(bytes("test", "utf-8")))
decode_dll.decode.restype = c_float
px = decode_dll.decode(s, c)


## 调用decode_message
decode_dll.decode_message.restype = c_char_p  # 设置返回类型
res = decode_dll.decode_message(c)
res = res.decode()  # bytes -> str

```

- 附图  
[官方链接](https://docs.python.org/zh-cn/3.7/library/ctypes.html)

<div class="section" id="fundamental-data-types">
<span id="ctypes-fundamental-data-types"></span><h3>基础数据类型<a class="headerlink" href="#fundamental-data-types" title="永久链接至标题">¶</a></h3>
<p><a class="reference internal" href="#module-ctypes" title="ctypes: A foreign function library for Python."><code class="xref py py-mod docutils literal notranslate"><span class="pre">ctypes</span></code></a> 定义了一些和C兼容的基本数据类型：</p>
<table class="docutils align-default">
<colgroup>
<col style="width: 24%">
<col style="width: 46%">
<col style="width: 30%">
</colgroup>
<thead>
<tr class="row-odd"><th class="head"><p>ctypes 类型</p></th>
<th class="head"><p>C 类型</p></th>
<th class="head"><p>Python 类型</p></th>
</tr>
</thead>
<tbody>
<tr class="row-even"><td><p><a class="reference internal" href="#ctypes.c_bool" title="ctypes.c_bool"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_bool</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">_Bool</span></code></p></td>
<td><p>bool (1)</p></td>
</tr>
<tr class="row-odd"><td><p><a class="reference internal" href="#ctypes.c_char" title="ctypes.c_char"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_char</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">char</span></code></p></td>
<td><p>单字符字节对象</p></td>
</tr>
<tr class="row-even"><td><p><a class="reference internal" href="#ctypes.c_wchar" title="ctypes.c_wchar"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_wchar</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">wchar_t</span></code></p></td>
<td><p>单字符字符串</p></td>
</tr>
<tr class="row-odd"><td><p><a class="reference internal" href="#ctypes.c_byte" title="ctypes.c_byte"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_byte</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">char</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-even"><td><p><a class="reference internal" href="#ctypes.c_ubyte" title="ctypes.c_ubyte"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_ubyte</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">unsigned</span> <span class="pre">char</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-odd"><td><p><a class="reference internal" href="#ctypes.c_short" title="ctypes.c_short"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_short</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">short</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-even"><td><p><a class="reference internal" href="#ctypes.c_ushort" title="ctypes.c_ushort"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_ushort</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">unsigned</span> <span class="pre">short</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-odd"><td><p><a class="reference internal" href="#ctypes.c_int" title="ctypes.c_int"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_int</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">int</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-even"><td><p><a class="reference internal" href="#ctypes.c_uint" title="ctypes.c_uint"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_uint</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">unsigned</span> <span class="pre">int</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-odd"><td><p><a class="reference internal" href="#ctypes.c_long" title="ctypes.c_long"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_long</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">long</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-even"><td><p><a class="reference internal" href="#ctypes.c_ulong" title="ctypes.c_ulong"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_ulong</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">unsigned</span> <span class="pre">long</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-odd"><td><p><a class="reference internal" href="#ctypes.c_longlong" title="ctypes.c_longlong"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_longlong</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">__int64</span></code> 或 <code class="xref c c-type docutils literal notranslate"><span class="pre">long</span> <span class="pre">long</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-even"><td><p><a class="reference internal" href="#ctypes.c_ulonglong" title="ctypes.c_ulonglong"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_ulonglong</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">unsigned</span> <span class="pre">__int64</span></code> 或 <code class="xref c c-type docutils literal notranslate"><span class="pre">unsigned</span> <span class="pre">long</span> <span class="pre">long</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-odd"><td><p><a class="reference internal" href="#ctypes.c_size_t" title="ctypes.c_size_t"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_size_t</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">size_t</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-even"><td><p><a class="reference internal" href="#ctypes.c_ssize_t" title="ctypes.c_ssize_t"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_ssize_t</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">ssize_t</span></code> 或 <code class="xref c c-type docutils literal notranslate"><span class="pre">Py_ssize_t</span></code></p></td>
<td><p>整型</p></td>
</tr>
<tr class="row-odd"><td><p><a class="reference internal" href="#ctypes.c_float" title="ctypes.c_float"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_float</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">float</span></code></p></td>
<td><p>浮点数</p></td>
</tr>
<tr class="row-even"><td><p><a class="reference internal" href="#ctypes.c_double" title="ctypes.c_double"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_double</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">double</span></code></p></td>
<td><p>浮点数</p></td>
</tr>
<tr class="row-odd"><td><p><a class="reference internal" href="#ctypes.c_longdouble" title="ctypes.c_longdouble"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_longdouble</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">long</span> <span class="pre">double</span></code></p></td>
<td><p>浮点数</p></td>
</tr>
<tr class="row-even"><td><p><a class="reference internal" href="#ctypes.c_char_p" title="ctypes.c_char_p"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_char_p</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">char</span> <span class="pre">*</span></code> (以 NUL 结尾)</p></td>
<td><p>字节串对象或 <code class="docutils literal notranslate"><span class="pre">None</span></code></p></td>
</tr>
<tr class="row-odd"><td><p><a class="reference internal" href="#ctypes.c_wchar_p" title="ctypes.c_wchar_p"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_wchar_p</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">wchar_t</span> <span class="pre">*</span></code> (以 NUL 结尾)</p></td>
<td><p>字符串或 <code class="docutils literal notranslate"><span class="pre">None</span></code></p></td>
</tr>
<tr class="row-even"><td><p><a class="reference internal" href="#ctypes.c_void_p" title="ctypes.c_void_p"><code class="xref py py-class docutils literal notranslate"><span class="pre">c_void_p</span></code></a></p></td>
<td><p><code class="xref c c-type docutils literal notranslate"><span class="pre">void</span> <span class="pre">*</span></code></p></td>
<td><p>int 或 <code class="docutils literal notranslate"><span class="pre">None</span></code></p></td>
</tr>
</tbody>
</table>

