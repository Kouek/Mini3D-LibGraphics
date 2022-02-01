# 坐标系统 与 矩阵运算

[toc]

需要用到**LibGraphics**写C大程项目，相信你一定已经学完~~并忘掉~~**线性代数**了。

不过没关系，本项目用到的数学知识特别简单，将全部在下文讲完。三维图形算法中一般只用到 $4\cross3$ 或 $4\cross4$ 的矩阵，以及 $4\cross1$ 或 $3\cross1$ 的向量。

## 左右手系 与 矩阵、向量的计算机表示

在本项目中，将统一使用**右手坐标系**和**行主序表示**。

- 下图中，左边为**右手系**，右边为**左手系**。
  - 弯曲箭头表示使用**右手**或**左手**为 $[1,0,0]\cross[0,1,0]$ 的结果确定方向。
    - 试一试手握拳，让拇指以外的手指与**红、绿箭头**重合，看看拇指是否指向**蓝色箭头**的方向。这就是如此命名的原因！

![](./rsc/CoordAndMatrix-RLCoord.png)

- **行主序**与**列主序**则表示矩阵在计算机内存中的布局。
  - 序号表示内存偏移量。如行主序矩阵中， $mat4\cross4[6]$ 表示 $a_{12}$（行列均从0开始计数）。

```c
// 行主序
// Matrix 4x4    | Vector 4x1
// +--+--+--+--+ | +-+
// |0 |1 |2 |3 | | |0|
// +--+--+--+--+ | +-+
// |4 |5 |6 |7 | | |1|
// +--+--+--+--+ | +-+
// |8 |9 |10|11| | |2|
// +--+--+--+--+ | +-+
// |12|13|14|15| | |3|
// +--+--+--+--+ | +-+

// 列主序
// Matrix 4x4    | Vector 4x1
// +--+--+--+--+ | +-+
// |0 |4 |8 |12| | |0|
// +--+--+--+--+ | +-+
// |1 |5 |9 |13| | |1|
// +--+--+--+--+ | +-+
// |2 |6 |10|14| | |2|
// +--+--+--+--+ | +-+
// |3 |7 |11|15| | |3|
// +--+--+--+--+ | +-+
```

## 矩阵点乘

我们只需要实现 $4\cross4$ 的矩阵和 $4\cross1$ 的向量之间的点乘即可（至于为什么，见后文）。

- $4\cross4$ 矩阵点乘规则（矩阵点乘向量同理）


$$
\begin{aligned}

&A_{4\cross4}*B_{4\cross4}\\
&=
\left[
\begin{matrix}
a_{00} & a_{01} & a_{02} & a_{03} \\
a_{10} & a_{11} & a_{12} & a_{13} \\
a_{20} & a_{21} & a_{22} & a_{23} \\
a_{30} & a_{31} & a_{32} & a_{33}
\end{matrix}
\right]
*
\left[
\begin{matrix}
b_{00} & b_{01} & b_{02} & b_{03} \\
b_{10} & b_{11} & b_{12} & b_{13} \\
b_{20} & b_{21} & b_{22} & b_{23} \\
b_{30} & b_{31} & b_{32} & b_{33}
\end{matrix}
\right]\\
&=
\left[
\begin{matrix}
a_{00}b_{00}+a_{01}b_{10}+a_{02}b_{20}+a_{03}b_{30} & a_{00}b_{01}+a_{01}b_{11}+a_{02}b_{21}+a_{03}b_{31}& \cdots & \cdots \\
a_{10}b_{00}+a_{11}b_{10}+a_{12}b_{20}+a_{13}b_{30} & a_{10}b_{01}+a_{11}b_{11}+a_{12}b_{21}+a_{13}b_{31} & \cdots & \cdots \\
\vdots & \vdots & \ddots & \vdots \\
\cdots & \cdots & \cdots & a_{30}b_{03}+a_{31}b_{13}+a_{32}b_{23}+a_{33}b_{33}
\end{matrix}
\right]

\end{aligned}
$$

- 在看实现之前，我们先看一下磁盘的文件结构：
  - 相比上一章节，添加了**math3D.h**文件。

```
+ <项目名>
|__+ libragphics
|  |__ win32Export.h 
|  |__ graphics.h
|  |__ graphics.c
|  |__ ...
|__+ libg3D
|  |__ graphics3D.h
|  |__ graphics3D.c
|  |__ math3D.h
|__ main.c
|__ <项目名>.sln
|__ <项目名>.vcxproj
|__ ...
```

- 实现

```c
// math3D.h
// 由于笔者想偷懒，所以函数实现都写在.h文件中，函数需要加static防止符号重定义问题
// 同时，添加inline建议编译器“尽量”把函数嵌到调用者的代码中，减少函数调用

// 使用宏减少重复编码
#define MUL_THEN_ADD(lft, rht, row, col, stride) \
	lft[row * 4] * rht[col] \
	+ lft[row * 4 + 1] * rht[col + stride] \
	+ lft[row * 4 + 2] * rht[col + stride * 2] \
	+ lft[row * 4 + 3] * rht[col + stride * 3]

static inline void libg3DMat4x4MulVec4(float* outVec4, const float* lftMat4x4, const float* rhtVec4)
{
	// 使用临时变量v4，避免 outVec4==rhtVec4 时的原地运算问题
	float v4[4];
	v4[0] = MUL_THEN_ADD(lftMat4x4, rhtVec4, 0, 0, 1);
	v4[1] = MUL_THEN_ADD(lftMat4x4, rhtVec4, 1, 0, 1);
	v4[2] = MUL_THEN_ADD(lftMat4x4, rhtVec4, 2, 0, 1);
	v4[3] = MUL_THEN_ADD(lftMat4x4, rhtVec4, 3, 0, 1);
	memcpy(outVec4, v4, sizeof(float) * 4);
}

static inline void libg3DMat4x4MulMat4x4(float* outMat4x4, const float* lftMat4x4, const float* rhtMat4x4)
{
	// 使用临时变量m4，避免 outMat4x4==lft/rhtMar4x4 时的原地运算问题
	float m4[16];
	m4[0] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 0, 0, 4);
	m4[1] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 0, 1, 4);
	m4[2] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 0, 2, 4);
	m4[3] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 0, 3, 4);
	m4[4] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 1, 0, 4);
	m4[5] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 1, 1, 4);
	m4[6] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 1, 2, 4);
	m4[7] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 1, 3, 4);
	m4[8] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 2, 0, 4);
	m4[9] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 2, 1, 4);
	m4[10] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 2, 2, 4);
	m4[11] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 2, 3, 4);
	m4[12] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 3, 0, 4);
	m4[13] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 3, 1, 4);
	m4[14] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 3, 2, 4);
	m4[15] = MUL_THEN_ADD(lftMat4x4, rhtMat4x4, 3, 3, 4);
	memcpy(outMat4x4, m4, sizeof(float) * 16);
}

#undef MUL_THEN_ADD // 个人习惯，防止宏重定义
```

## 单位矩阵（Identity or Einheit）

学线性代数时最喜欢碰见的就是**单位矩阵**，用 $I$ 或 $E$ 表示，一般有如下性质：

- 长这样

$$
I=
\left[
\begin{matrix}
1 & 0 & \cdots & 0\\
0 & 1 & \cdots & 0\\
\vdots & \vdots & \ddots & \vdots\\
0 & 0 & \cdots & 1\\
\end{matrix}
\right]
$$

- 对于任意 $A_{m\cross m}$，有

$$
\begin{aligned}

&I_{m\cross m}*A_{m\cross m}\\
&=
\left[
\begin{matrix}
1 & 0 & 0 & \cdots & 0\\
0 & 1 & 0 & \cdots & 0\\
0 & 0 & 1 & \cdots & 0\\
\vdots & \vdots & \vdots & \ddots & \vdots\\
0 & 0 & 0 & \cdots & 1\\
\end{matrix}
\right]
*
\left[
\begin{matrix}
a_{00} & a_{01} & a_{02} & a_{03} \\
a_{10} & a_{11} & a_{12} & a_{13} \\
a_{20} & a_{21} & a_{22} & a_{23} \\
a_{30} & a_{31} & a_{32} & a_{33}
\end{matrix}
\right]\\
&=
\left[
\begin{matrix}
a_{00} & a_{01} & a_{02} & a_{03} \\
a_{10} & a_{11} & a_{12} & a_{13} \\
a_{20} & a_{21} & a_{22} & a_{23} \\
a_{30} & a_{31} & a_{32} & a_{33}
\end{matrix}
\right]\\
&=A

\end{aligned}
$$

- 还有

$$
A_{m\cross m}*I_{m\cross m}=\ ...\ =A
$$

- 对于任意代表**空间中三维坐标**的向量 $p=[x,y,z]^T$ 也同理

$$
I_{3\cross3}*p_{3\cross1}=
\left[
\begin{matrix}
1 & 0 & 0\\
0 & 1 & 0\\
0 & 0 & 1\\
\end{matrix}
\right]
*
\left[
\begin{matrix}
x\\
y\\
z\\
\end{matrix}
\right]
=
\left[
\begin{matrix}
x\\
y\\
z\\
\end{matrix}
\right]
=p
\\\\
p_{3\cross1}*I_{3\cross3}=\ ...\ =p
$$

显然，**单位阵**左乘右乘矩阵、向量，都不会对其做出改变。虽然看起来没什么用，但正是这种**不变性**让我们安心。

因此，若要我们提供一个**分配内存 并 构造矩阵**的接口，提供**单位阵**是第一需求。

- 实现

```c
// math3D.h

#define SET_IDENTITY4X4(mat) \
	memset(##mat, 0, sizeof(float) * 16);\
	##mat[0] = ##mat[5] = ##mat[10] = ##mat[15] = 1.f;

static inline float* libg3DGenIdentity4x4()
{
	float* mat = NULL;
	mat = malloc(sizeof(float) * 16);
	SET_IDENTITY4X4(mat);
	return mat;
}

static inline void libg3DSetIdentity4x4(float* mat4x4)
{
	SET_IDENTITY4X4(mat4x4);
}

#undef SET_IDENTITY4X4
```

## 缩放矩阵

**缩放矩阵**是最简单的变换矩阵。当我们要将向量 $p=[x,y,z]^T$ 以原点为中心（很多软件管这个叫**锚点**，比如Adobe系列）缩放到$p'=[s_xx,s_yy,s_zz]^T$ 时，显然可以用下面的矩阵：
$$
\begin{aligned}

&\because
\left[
\begin{matrix}
s_x & 0 & 0\\
0 & s_y & 0\\
0 & 0 & s_z\\
\end{matrix}
\right]
*
p
=
\left[
\begin{matrix}
s_x & 0 & 0\\
0 & s_y & 0\\
0 & 0 & s_z\\
\end{matrix}
\right]
*
\left[
\begin{matrix}
x\\
y\\
z\\
\end{matrix}
\right]
=
\left[
\begin{matrix}
s_xx\\
s_yy\\
s_zz\\
\end{matrix}
\right]=p'\\

&\therefore
Scale_{xyz}
=
\left[
\begin{matrix}
s_x & 0 & 0\\
0 & s_y & 0\\
0 & 0 & s_z\\
\end{matrix}
\right]

\end{aligned}
$$

- 实现

```c
// math3D.h

static inline void libg3DMat4x4Scale(float* inOutMat4x4, float sX, float sY, float sZ)
{
	// 等价于左乘 [[sX,0,0,0], [0,sY,0,0], [0,0,sZ,0], [0,0,0,1]]。
	inOutMat4x4[0] *= sX;
	inOutMat4x4[1] *= sX;
	inOutMat4x4[2] *= sX;
	inOutMat4x4[3] *= sX;
	inOutMat4x4[4] *= sY;
	inOutMat4x4[5] *= sY;
	inOutMat4x4[6] *= sY;
	inOutMat4x4[7] *= sY;
	inOutMat4x4[8] *= sZ;
	inOutMat4x4[9] *= sZ;
	inOutMat4x4[10] *= sZ;
	inOutMat4x4[11] *= sZ;
}
```

## 旋转矩阵

**旋转矩阵**是推导比较麻烦的一个矩阵。一般都从**绕 $z$ 轴旋转**入手，再推广到绕 $y$ 轴和绕 $x$ 轴旋转。

如果我们现在要将向量 $p$ 绕 $z$ 轴逆时针旋转 $d\alpha$ 角到 $p'$，有
$$
d\alpha=a_{P'}-a_P
$$
![](./rsc/CoordAndMatrix-RotateXOY.png)

由于旋转保长度，假设 $\Vert p\Vert_2=L$，且其 $z$ 轴分量为 $z$，有
$$
\begin{aligned}

\because\\
p&=[L\cos{\alpha_p},\ L\sin{\alpha_p},\ z]^T\\
\and\\
p'&=[L\cos{\alpha_{p'}},\ L\sin{\alpha_{p'}},\ z]^T\\
&=[L\cos{(\alpha_p+d\alpha)},\ L\sin{(\alpha_p+d\alpha)},\ z]^T\\
&=[L\cos{(\alpha_p+d\alpha)},\ L\sin{(\alpha_p+d\alpha)},\ z]^T\\
&=[L(\cos{\alpha_p}\cos{d\alpha}-\sin{\alpha_p}\sin{d\alpha}),\ L(\sin{\alpha_p}\cos{d\alpha}+\cos{\alpha_p}\sin{d\alpha}),\ z]^T\\

\therefore\\
p'&=
\left[
\begin{matrix}
\cos{d\alpha} & -\sin{d\alpha} & 0 & 0\\
\sin{d\alpha} & \cos{d\alpha} & 0 & 0\\
0 & 0 & 1 & 0\\
0 & 0 & 0 & 1
\end{matrix}
\right]
*[L\cos{\alpha_p},\ L\sin{\alpha_p},\ z]^T\\&=Rotate_z(d\alpha)*p

\end{aligned}
$$
可以看到，**旋转矩阵**只与转过的角 $d\alpha$ 有关。

由于把上图的 $xOy$ 换成 $yOz$ 或 $zOx$ 也适用，因此可以很快地推导出绕 $y$ 和绕 $x$ 的**旋转矩阵**。
$$
\begin{aligned}

Rotate_y(d\alpha)&=
\left[
\begin{matrix}
\cos{d\alpha} & 0 & \sin{d\alpha} & 0\\
0 & 1 & 0 & 0\\
-\sin{d\alpha} & 0 & \cos{d\alpha} & 0\\
0 & 0 & 0 & 1
\end{matrix}
\right]\\

Rotate_x(d\alpha)&=
\left[
\begin{matrix}
1 & 0 & 0 & 0\\
0 & \cos{d\alpha} & -\sin{d\alpha} & 0\\
0 & \sin{d\alpha} & \cos{d\alpha} & 0\\
0 & 0 & 0 & 1
\end{matrix}
\right]

\end{aligned}
$$
要在三维空间中旋转到一个你喜欢的角度，可以先按 $z$ 轴旋转 $d\alpha_z$（左乘 $Rotate_z$），再按 $y$ 轴旋转 $d\alpha_y$（左乘 $Rotate_y$），最后按 $x$ 轴旋转 $d\alpha_x$（左乘 $Rotate_x$）。
$$
\begin{aligned}

Rotate&=Rotate_x*Rotate_y*Roatate_z\\
&=
\left[
\begin{matrix}
1 & 0 & 0 & 0\\
0 & \cos{d\alpha_x} & -\sin{d\alpha_x} & 0\\
0 & \sin{d\alpha_x} & \cos{d\alpha_x} & 0\\
0 & 0 & 0 & 1
\end{matrix}
\right]
*
\left[
\begin{matrix}
\cos{d\alpha_y} & 0 & \sin{d\alpha_y} & 0\\
0 & 1 & 0 & 0\\
-\sin{d\alpha_y} & 0 & \cos{d\alpha_y} & 0\\
0 & 0 & 0 & 1
\end{matrix}
\right]
*
\left[
\begin{matrix}
\cos{d\alpha_z} & -\sin{d\alpha_z} & 0 & 0\\
\sin{d\alpha_z} & \cos{d\alpha_z} & 0 & 0\\
0 & 0 & 1 & 0\\
0 & 0 & 0 & 1
\end{matrix}
\right]\\
&=
\left[
\begin{matrix}
\cos{d\alpha_y} & 0 & \sin{d\alpha_y} & 0\\
\sin{d\alpha_x}\sin{d\alpha_y} & \cos{d\alpha_x} & -\sin{d\alpha_x}\cos{d\alpha_y} & 0\\
-\cos{d\alpha_x}\sin{d\alpha_y} & \sin{d\alpha_x} & \cos{d\alpha_x}\cos{d\alpha_y} & 0\\
0 & 0 & 0 & 1
\end{matrix}
\right]
*
\left[
\begin{matrix}
\cos{d\alpha_z} & -\sin{d\alpha_z} & 0 & 0\\
\sin{d\alpha_z} & \cos{d\alpha_z} & 0 & 0\\
0 & 0 & 1 & 0\\
0 & 0 & 0 & 1
\end{matrix}
\right]\\
&=
\left[
\begin{matrix}
\cos{d\alpha_y}\cos{d\alpha_z} & -\cos{d\alpha_x}\sin{d\alpha_z} & \sin{d\alpha_y} & 0\\
\sin{d\alpha_x}\sin{d\alpha_y}\cos{d\alpha_z}+\cos{d\alpha_x}\sin{d\alpha_z} & -\sin{d\alpha_x}\sin{d\alpha_y}\sin{d\alpha_z}+\cos{d\alpha_x}\cos{d\alpha_z} & -\sin{d\alpha_x}\cos{d\alpha_y} & 0\\
-\cos{d\alpha_x}\sin{d\alpha_y}\cos{d\alpha_z}+\sin{d\alpha_x}\sin{d\alpha_z} & \cos{d\alpha_x}\sin{d\alpha_y}\sin{d\alpha_z}+\sin{d\alpha_x}\cos{d\alpha_z} & \cos{d\alpha_x}\cos{d\alpha_y} & 0\\
0 & 0 & 0 & 1
\end{matrix}
\right]

\end{aligned}
$$
根据上面的公式，我们可以写出对应的代码。

- 实现

```c
// math3D.h

#define pI 3.1415926f
#define DEG_TO_RAD pI/180.f

static inline void libg3DMat4x4Rotate(float* inOutMat4x4, float degX, float degY, float degZ)
{
	float alphas[] = {
		degX * DEG_TO_RAD,
		degY * DEG_TO_RAD,
		degZ * DEG_TO_RAD
	};
	float coss[] = {
		cosf(alphas[0]),
		cosf(alphas[1]),
		cosf(alphas[2])
	};
	float sins[] = {
		sinf(alphas[0]),
		sinf(alphas[1]),
		sinf(alphas[2])
	};
	float left[16] = { 0 };
	left[0] = coss[1] * coss[2];
	left[1] = -coss[0] * sins[2];
	left[2] = sins[1];
	left[4] = sins[0] * sins[1] * coss[2] + coss[0] * sins[2];
	left[5] = coss[0] * coss[2] - sins[0] * sins[1] * sins[2];
	left[6] = -sins[0] * coss[1];
	left[8] = sins[0] * sins[2] - coss[0] * sins[1] * coss[2];
	left[9] = coss[0] * sins[1] * sins[2] + sins[0] * coss[2];
	left[10] = coss[0] * coss[1];
	left[15] = 1;
	libg3DMat4x4MulMat4x4(inOutMat4x4, left, inOutMat4x4);
}
```

## 平移矩阵

把**平移矩阵**放到最后，不是因为它复杂，而是因为它需要引入**齐次坐标**这个概念（虽然我们已经在代码中用了）。

众所周知（bushi），$3\cross3$ 的矩阵表示三维欧氏空间的线性变换。

一个变换 $f(x),x\in R^n$ 是 $n$ 维欧氏空间 $R^n$ 的线性变换，当且仅当
$$
\begin{aligned}

f(x+y)&=f(x)+f(y)\\
&\and\\
f(kx)&=kf(x)\\
&&where\ x,y\in R^n,\ k\in R\and k\neq0

\end{aligned}
$$
假设存在一个线性变换 $t(p),p\in R^3$ 使得**点** $p=[x,y,z]^T$ 平移到 $p'=[x+t_x,y+t_y,z+t_z]^T$（注意，这里只能对点进行平移，因为向量有平移不变性） ，则有
$$
\begin{aligned}

\because \ t(p)&=p'=p+[t_x,t_y,t_z]^T\\

\therefore \ t(p+p)&=p+p+[t_x,t_y,t_z]^T\\
&\neq p+[t_x,t_y,t_z]^T+p+[t_x,t_y,t_z]^T\\
&=t(p)+t(p)

\end{aligned}
$$
所以不存在该线性变换，即**不存在矩阵 $T_{3\cross3}$ 使得 $Tp=p'$**。

而如果我们给点 $p$ 升一维，使用以下对应关系进行转化：
$$
\begin{aligned}

p_3\to p_4&=[x,y,z]^T\to[x,y,z,1]^T\\
p_4\to p_3&=[x,y,z,w]^T\to[\frac{x}{w},\frac{y}{w},\frac{z}{w}]^T

\end{aligned}
$$
则可以有以下论证：

要将 $R^3$ 下的 $p=[x,y,z]^T$ 平移到 $p'=[x+t_x,y+t_y,z+t_z]^T$，即将 $R^4$ 下的 $p=[x,y,z,1]^T$ 变换到 $p'=[t_w(x+t_x),t_w(y+t_y),t_w(z+t_z),t_w]^T,t_w\neq 0$。要实现这个变换，显然可以用下面的矩阵：
$$
\begin{aligned}

&\because
\left[
\begin{matrix}
1 & 0 & 0 & t_x\\
0 & 1 & 0 & t_x\\
0 & 0 & 1 & t_x\\
0 & 0 & 0 & 1\\
\end{matrix}
\right]
*
\left[
\begin{matrix}
x\\y\\z\\1
\end{matrix}
\right]
=
\left[
\begin{matrix}
x+t_x\\y+t_y\\z+t_z\\1
\end{matrix}
\right]\\

&\therefore
Translate=
\left[
\begin{matrix}
1 & 0 & 0 & t_x\\
0 & 1 & 0 & t_x\\
0 & 0 & 1 & t_x\\
0 & 0 & 0 & 1\\
\end{matrix}
\right]

\end{aligned}
$$
此时 $t_w=1$。

- 实现

```c
static inline void libg3DMat4x4Translate(float* inOutMat4x4, float tX, float tY, float tZ)
{
	// 等价于左乘 [[1,0,0,tX], [0,1,0,tY], [0,0,1,tZ], [0,0,0,1]]。
	//   由于未经透视投影前，不管如何变换，第3行均为[0,0,0,1]，
	//   可不用左乘，直接简化如下
	inOutMat4x4[3] += tX;
	inOutMat4x4[7] += tY;
	inOutMat4x4[11] += tZ;
}
```

## 综合一下——位姿矩阵（Pose）

已经实现了缩放、旋转、平移变换，那要怎么把他们结合起来呢？这其实要看你的需求。

一个常见的需求是描述一个**相机**。首先，相机无所谓大小缩放，它在渲染器中一般用来实现视角变换，只需要有**位置**和**方向**两个属性就足够了。视角的放大缩小可以通过更改后文的**投影参数**实现。

其次，一般情况下，相机的位置和方向需要相对**同一个相对固定参照系**来描述。在拍摄现场，导演一般会说：“把相机弄到桌子后x米，朝向桌子”，而很少说：“把相机弄到桌子后x米，到了之后再朝左边转x度，朝下边转x度”。



## 投影矩阵



## 综合起来——模型、观察、投影矩阵（MVP）

$$
\begin{aligned}

MVP&=Projection*View*Model\\

\end{aligned}
$$

