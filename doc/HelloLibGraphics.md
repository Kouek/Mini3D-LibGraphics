# 环境配置 与 Hello LibGraphics

**LibGraphics**是只面向**Windows**的图形库，因此本项目使用~~朴实无华的~~**Visual Studio**进行开发。C大程授课一般使用VS 2010版本，但由于笔者懒得去下载旧版，就用手头的VS 2019。

**VS 2019**要跑起**LibGraphics**来还是需要一番配置的。

- 打开VS 2019，创建一个**空的C/C++项目**。

- 把[刘新国老师](https://person.zju.edu.cn/xgliu)维护的**LibGraphics**下载到本地，将其中的**libgraphics**文件夹复制到项目在磁盘的根目录下。

- 在VS 2019 的 解决方案资源管理器 - \<项目名\>/源文件 中，添加一个**筛选器**（就是虚拟的文件夹），命名为**libgraphics**。

- 对 \<项目名\>/源文件/libgraphics 右键，添加现有项，将**libgraphics**文件夹（在磁盘的项目根目录下）中所有的**.c后缀**文件添加进来。

- 对 \<项目名\>/源文件/libgraphics 右键，添加新建项，添加一个**main.c**文件。

- 以上步骤之后，磁盘中的项目根目录如下：


```
+ <项目名>
|__+ libragphics
|  |__ graphics.h
|  |__ graphics.c
|  |__ ...
|__ main.c
|__ <项目名>.sln
|__ <项目名>.vcxproj
|__ ...
```

- VS 2019中的解决方案资源管理器如下：


```
+ <项目名>
|__+ 头文件
|__+ 源文件
|  |__ main.c
|  |__+ libgraphics
|     |__ graphics.c
|     |__ genlib.c
|     |__ ...
|__ ...
```

- 在**main.c**中输入以下代码：


```c
#include "libgraphics/extgraph.h"

void Main()
{
	InitGraphics();
	SetWindowTitle("Hello LibGraphics");
	MovePen(0, 0);
	DrawLine(5, 5);
} 
```

- 按F5编译并调试，肯定会输出如下错误：

  - > C4996 ......This function or variable may be unsafe. Consider using <\*\*\*> instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.

- 参考错误提示信息，对VS 2019的 解决方案资源管理器 - \<项目名\> 右键，进入 属性，在 配置属性 - C/C++ - 预处理器 中，为**预处理器定义**栏添加**_CRT_SECURE_NO_WARNINGS**。

- 按F5编译并调试，之前的错误已经消失，但肯定会输出如下错误：

  - > LNK2019	无法解析的外部符号 _main，函数 "int __cdecl invoke_main(void)" (?invoke_main@@YAHXZ) 中引用了该符号

- 参考[链接器工具错误 LNK2019 | Microsoft Docs](https://docs.microsoft.com/zh-cn/cpp/error-messages/tool-errors/linker-tools-error-lnk2019?view=msvc-170#you-build-a-console-application-by-using-settings-for-a-windows-application)，同上进入项目的属性配置窗口，将 配置属性 - 链接器 - 系统 的**子系统**选项从**控制台**改成**窗口**。

- 按F5编译并调试，之前的错误已经消失，但*可能*会输出如下错误：

  - > 由于找不到\<\*\*\*\>.dll，无法继续执行代码，......

- 根据笔者的经验（Doge），同上进入项目的属性配置窗口，将 配置属性 - C/C++ - 代码生成 的**运行库**从**多线程调试（MDd）**改成**多线程（MD）**。

- 按F5编译并调试，终于看到窗口弹出来了，其中有一条直线。这就是我们的**Hello LibGraphics程序**，用于验证开发环境的完整性。


![](./rsc/HelloLibGraphics-WrongCoding.png)

- 虽然如此，但窗口标题是乱码令人不爽。Debug发现是经典的字符集问题。

  - Debug过程，一路F12寻找符号的定义：

```c
// main.c
SetWindowTitle("Hello LibGraphics"); // 对它【按F12】
// ||
// \/
// extgraph.h
void SetWindowTitle(string title)
{
    windowTitle = CopyString(title);
    if (initialized) {
        SetWindowText(graphicsWindow, windowTitle); // 对它【按F12】
    }
}
// ||
// \/
// WinUser.h
BOOL
WINAPI
SetWindowTextA(
    _In_ HWND hWnd,
    _In_opt_ LPCSTR lpString);
WINUSERAPI
BOOL
WINAPI
SetWindowTextW(
    _In_ HWND hWnd,
    _In_opt_ LPCWSTR lpString);
#ifdef UNICODE // 发现UNICODE宏被定义，调用SetWindowTextW，因此对其参数2 LPCWSTR 【按F12】
#define SetWindowText  SetWindowTextW
#else
#define SetWindowText  SetWindowTextA
#endif // !UNICODE
// ||
// \/
// winnt.h
typedef _Null_terminated_ CONST WCHAR *LPCWSTR, *PCWSTR; // WCHAR发现！
```

- 可以看到，由于**UNICODE宏**被定义，最终调用的是**SetWindowTextW**（一个WIN32的API），使用的参数是面向**Unicode字符集**的**WCHAR**，而LibGraphics使用的**string**（实际上是**char\***类型）只支持**ASCⅡ字符集**，我们传入的**"Hello LibGraphics"**也都是纯纯的ASCⅡ字符。

  - 得出最简单的解决方法：把**UNICODE宏**的定义取消！那这个宏定义在哪呢？
  - 对**UNICODE宏**按F12，VS 2019报告“找不到宏定义”。
    - **既然不在代码中定义，那肯定在预处理阶段定义了**。
  - 同上进入项目的属性配置窗口，查看 配置属性 - C/C++ 的**预处理器定义**栏，发现没有UNICODE。
  - 根据笔者的经验（Doge），查看 配置属性 - 高级，发现**字符集**选项为**使用UNICODE字符集**，将其改成**未设置**，问题解决！

![](./rsc/HelloLibGraphics.png)