# UE5 Slate

环境配置：UE5 5.6.0、Visual Studio 2020、JetBrains Rider 2025.1.3

## 1. Rider 下的 Project 目录

Rider 可以通过 **.sln**（双击打开编译器）、.uproject（双击打开 UE5）启动工程。两个启动方式的项目树结构不一样。

 `.sln` 和 `.uproject` 在 Rider 中的 **UnrealLink 配置是独立的**。

<img src="https://gitee.com/Koletis/pic-go/raw/master/202506191243182.png" alt="image-20250619124302786" style="zoom:80%;" />

`.sln`：

**1. `Engine`（引擎核心）**： 包含 **Unreal Engine 的核心代码**（如渲染、物理、AI、蓝图系统等）。  

- `UE5`（主引擎模块） 、`UnrealEditor`（编辑器核心）。
- 用户一般不需要修改，除非你要 **定制引擎底层功能**（如修改渲染管线）。

**2. `Games`（游戏项目）**：存放 **你的游戏项目代码**（如 `YourGame` 和 `YourGameEditor`）。  

- `YourGame`（游戏运行时逻辑）、`YourGameEditor`（游戏编辑器扩展，如自定义工具栏）。
- ✅用户需要修改，这是你主要编写代码的地方（如角色、关卡逻辑）。

**3. `Programs`（工具程序）**：包含 **UE 相关的工具和命令行程序**（如编译工具、资源处理器）。  

- `UnrealHeaderTool`（UHT，负责生成反射代码）、`ShaderCompileWorker`（着色器编译工具）、`SwarmAgent`（分布式构建工具）。
- 用户不需要修改，除非你要开发 UE 的配套工具。

**4. `Rules`（构建规则）**：定义 **模块的编译规则和依赖关系**（`.Target.cs` 和 `.Build.cs` 文件）。  

- `YourGame.Target.cs`（配置游戏构建目标）、`UnrealBuildTool`（UE 的构建系统核心）。
- 用户偶尔需要修改（如添加第三方库依赖或修改编译选项）。

**5. `Visualizers`（调试可视化工具）**：为 Visual Studio 调试器提供 **UE 特定类型的可视化支持**（如查看 `FVector` 的值）。  

- 用户一般不需要修改，除非你要扩展调试器功能。



`Games`：

<img src="https://gitee.com/Koletis/pic-go/raw/master/202506191250398.png" alt="image-20250619125023275" style="zoom:80%;" />

| **能力**              | `Config`         | `Plugins`     | `Source`            |
| --------------------- | ---------------- | ------------- | ------------------- |
| **修改现有菜单**      | ✅（仅调整显示）  | ❌             | ✅（但过度杀伤）     |
| **添加全新菜单/按钮** | ❌                | ✅             | ✅                   |
| **移植性**            | ✅（复制 `.ini`） | ✅（复制插件） | ❌（需手动迁移代码） |
| **复杂度**            | 低               | 中            | 高                  |



`Plugins`：

<img src="https://gitee.com/Koletis/pic-go/raw/master/202506191251686.png" alt="image-20250619125153571" style="zoom:80%;" />

**`.h`（头文件）**：声明类、函数、变量等，供其他代码引用。

**`.cpp`（实现文件）**：包含具体的实现逻辑，编译为二进制代码。

**`.Build.cs`（模块构建脚本）**：不参与编译，而是指导 Unreal Build Tool 如何处理 `.h` 和 `.cpp` 文件。



## 2. 插件、父模块、子模块

| **类类型**     | **基类**           | **用途**           | **典型函数**                          |
| -------------- | ------------------ | ------------------ | ------------------------------------- |
| 模块类         | `IModuleInterface` | 管理插件生命周期   | `StartupModule()`, `ShutdownModule()` |
| UI 组件类      | `SCompoundWidget`  | 构建可视化界面     | `Construct()`, `Tick()`               |
| 窗口类         | `SWindow`          | 创建独立窗口       | `SetContent()`, `ShowWindow()`        |
| UMG 用户界面类 | `UUserWidget`      | 基于 UMG 蓝图的 UI | `NativeConstruct()`, `NativeTick()`   |



插件：

```text
Plugins/SyntheticDataFactory/    # 插件
├── Resources/                   # 配置资源
├── Source/                      # 代码目录
│   ├── AssetSelector/           # 子模块
│   │   ├── Private/
│   │   ├── Public/
│   │   └── AssetSelector.Build.cs
│   └── SyntheticDataFactory/    # 主模块
│       ├── Private/
│       ├── Public/
│       └── SyntheticDataFactory.Build.cs
└── SyntheticDataFactory.uplugin  # 插件描述文件
```



模块：

```text
AssetSelector/                   # 子模块
├── Private/
│   ├── AssetSelector.cpp        # 模块类（必选）
│   └── SAssetSelector.cpp       # 控件类（可选）
├── Public/
│   ├── AssetSelector.h          # 模块类
│   └── SAssetSelector.h         # 控件类
└── AssetSelector.Build.cs       # 模块构建配置
```



<mark>无论是主模块还是子模块（所有模块），都需要：</mark>

1. 定义一个继承自 `IModuleInterface` 的类。
2. 实现 `StartupModule()` 和 `ShutdownModule()` 方法。
3. 使用 `IMPLEMENT_MODULE` 宏注册模块。
4. 在 `.uproject` 的 `"Modules"` 中加入新模块。
5. 在需要使用的模块 `.Build.cs` 中加入新模块名。

注： *3* 结构体不需要。



主模块要使用子模块的方法（跨模块操作），子模块需要导出宏。具体操作：

1. 在子模块的公共头文件 `.h` 定义导出宏：`#if defined...`。
2. 在子模块的 `.Build.cs` 中启用导出：`PublicDefinitions.Add("...");`。
3. 在子模块的公共方法 `.cpp` 中使用导出宏：`XXX_API`。
4. 主模块直接调用子模块的导出接口（无需额外声明）。

注：前两步通过可以通过右击 `.uproject` 选择 `Generate Visual Studio project files` 自动配置，即不用手动添加。



## 3. 重新编译 UE5 项目

UE5 项目文件夹：

<img src="https://gitee.com/Koletis/pic-go/raw/master/202506301425905.png" alt="image-20250630142538268" style="zoom:80%;" />

**Binaries**

- **内容**：存放编译生成的可执行文件（如 `.exe`）、动态链接库（如 `.dll`、`.so`）、静态库（如 `.lib`、`.a`）以及其他运行时所需的二进制文件。
- **生成方式**：可删除。通过构建工具 Rider 编译项目（ `Build` / `Run` ）生成。

**Intermediate**

- **作用**：存放编译过程中生成的中间文件（如 `.obj`、`.o`、`.pch`）。
- **生成方式**：可删除。编译时自动生成，或右键点击 `.uproject` 选择 `Generate Visual Studio Project Files` 生成。



## 4. 定位控件源码位置

Tools > Debug > Widget Reflector

<img src="https://gitee.com/Koletis/pic-go/raw/master/202507071726396.png" alt="image-20250707172647936" style="zoom:80%;" />

选定控件后，按下 ESC 即可退出。

<img src="https://gitee.com/Koletis/pic-go/raw/master/202507071728865.png" alt="image-20250707172803694" style="zoom:80%;" />

双击 Source ，即可跳转到源码。



## 5. 基础控件样式

点击 UE 编辑器右下方 Trace > Unreal Insights (Session Browser) > 点击 Trace Store 右下方 “⌵” > Startship Test Suite

<img src="https://gitee.com/Koletis/pic-go/raw/master/202507071744234.png" alt="image-20250707174405040" style="zoom:80%;" />

STARSHIP GALLERY：

![image-20250707174313331](https://gitee.com/Koletis/pic-go/raw/master/202507071743616.png)



## 6. 学习

视频教程：https://www.bilibili.com/video/BV1Vt9aYpEdZ

### 6.1 工具

- Display UI Extension Points：显示 UI 拓展点，用于查看图形界面菜单在底层代码中的标识符。设置中勾选工具开启 02:32。
- bEnableEditToolMenusUl：启用编辑器工具菜单，用于查看扩展点的层级结构（类似路径）。编辑器控制台开启 03:42。

### 6.2 FExtender

1. 拓展主菜单

> 编辑器窗口的右上角

简易开发流程：**锚定位置（Position）→ 创建条目（Entry）→ 设置行为（Action）**。

- MainMenuExtender.AddMenuBarExtension：主菜单按钮
- MainMenuExtender.AddMenuExtension：主菜单按钮下的子按钮
- MainMenuExtender.AddToolBarExtension：Toolbar 按钮

4.2 拓展内容浏览器

> 内容浏览器（Content Browser）：默认位于编辑器底部，支持拖拽停靠或浮动，也可右键选择“移至侧边栏”折叠到左侧。

4.3 资产摘要信息

>  资产摘要信息（Summary）：显示在 Content Browser 的细节面板（Details Panel）或资产属性面板中

### 6.3 FExtender vs FToolMenus

**FExtender** 是菜单系统的 “装修队”，负责在已有框架上添加个性化功能。适合在已有菜单中插入自定义功能（如增强 “编辑” 菜单）。

**FToolMenus** 是菜单系统的 “地基”，负责定义和注册菜单的基础框架。适合创建全新的菜单系统（如插件专属菜单）。

| **特性**     | **FExtender**                  | **FToolMenus**                 |
| ------------ | ------------------------------ | ------------------------------ |
| **操作时机** | 运行时动态扩展（如插件加载时） | 初始化时静态注册（如模块启动） |
| **修改方式** | 不改变原有菜单结构，增量扩展   | 直接定义菜单的基础结构         |
| **适用场景** | 插件扩展、临时功能注入         | 菜单系统的基础搭建             |
| **依赖关系** | 依赖`FToolMenus`已注册的菜单   | 独立工作，提供扩展点           |

**何时应该只用 `FToolMenus`？**

- **场景 1**：创建全新的、独立的菜单系统（如插件专属菜单）。
- **场景 2**：添加固定不变的功能（如一键执行某个工具）。
- **场景 3**：开发内部工具，无需考虑与其他插件的兼容性。

### 6.4 UToolMenus 

<img src="https://gitee.com/Koletis/pic-go/raw/master/202506171026363.png" alt="image-20250617102613226" style="zoom:67%;" />

 UToolMenus > UToolMenu >  UToolMenuSection >  UToolMenuEntry

<img src="https://gitee.com/Koletis/pic-go/raw/master/202506170919456.png" alt="image-20250617091934269" style="zoom:80%;" />

![img](https://gitee.com/Koletis/pic-go/raw/master/202506171029786.png)

### 6.5 Widget（UMG）

基于 Slate 的**高层 UI 系统**，全称为 Unreal Motion Graphics。

核心类：**UWidget**

### 6.6 Slate 

> Slate 作为底层框架，能实现 FToolMenus 的所有功能，且扩展性更强。

UE 的底层 UI 渲染框架，类似 Unity 的 UGUI 或 Qt 的 Widgets。

核心类：**SWidget**

`FToolMenus` 不能直接调用 Slate 代码，但通过 `FMenuBuilder` 和 `FExtender`，可以无缝集成 Slate 控件到菜单系统中。

<img src="https://gitee.com/Koletis/pic-go/raw/master/202506171117134.png" alt="image-20250617111735998" style="zoom:80%;" />

```plaintext
FSlateApplication (管理)
    ↓
SWindow (容器)
    ↓
SWidget (基础元素)
```

### 6.7 Widget vs Slate

> Unreal Engine 5.5.4、IDE Rider

**Slate 是基础**：UMG 控件最终会编译为 Slate 控件执行。

**UMG 是封装**：简化 Slate 的复杂 API，提供更友好的开发方式。

| **特性**       | **Slate**                | **Widget（UMG）**              |
| -------------- | ------------------------ | ------------------------------ |
| **开发语言**   | C++                      | 蓝图为主，可结合 C++           |
| **可视化编辑** | 无（纯代码）             | ✅ 编辑器拖拽组件               |
| **性能**       | 高（底层优化）           | 中（自动生成代码）             |
| **动画支持**   | 需要手动实现             | ✅ 内置动画蓝图节点             |
| **适用范围**   | 编辑器 UI、高性能游戏 UI | 游戏内 UI（菜单、HUD、对话框） |

UMG (`UWidget`) 在运行时会被编译成 Slate (`SWidget`) 进行渲染，例如：

- `UButton` → 生成 `SButton`
- `UTextBlock` → 生成 `STextBlock`

### 6.8 实现流程

自定义菜单，并实现业务流程：

- **FToolMenus**：定义菜单项的**结构和位置**。
- **Slate/Widget**：实现菜单项的**视觉表现**。UI
- **FExtender**：**绑定 UI 和业务逻辑**。
- **业务逻辑**：通过 `FUIAction`、`UObject` 方法或 Slate 事件回调实现，通常调用现有模块或自定义功能。

![image-20250617113008266](https://gitee.com/Koletis/pic-go/raw/master/202506171130496.png)

| **步骤**     | **负责组件**      | **作用**                                                |
| ------------ | ----------------- | ------------------------------------------------------- |
| 菜单结构定义 | FToolMenus        | 注册菜单层级和基础条目                                  |
| 视觉渲染     | Slate             | 实现按钮样式、材质、动画等视觉效果                      |
| 位置绑定     | FExtender         | 确定菜单项在现有菜单中的位置，并插入 UI 元素            |
| 业务逻辑     | FUIAction / Slate | 绑定点击事件与具体功能实现（调用模块 API 或自定义函数） |

**总结**

该流程遵循 “**结构定义 → 视觉设计 → 绑定集成 → 逻辑实现**” 的分层原则，确保各组件职责清晰：`FToolMenus` 搭建菜单框架，`Slate` 负责外观，`FExtender` 连接两者，业务逻辑通过回调函数实现。



# C++

类外实现函数：

```cpp
// 类的声明（通常在头文件.h中）
class Model {
public:
    void Shutdown();  // 类内声明成员函数
};

// 类外定义成员函数（通常在源文件.cpp中）
void Model::Shutdown() {
    // 函数实现
}
```

派生类重写函数：

```cpp
class Base {  // 基类（父类）
public:
    virtual void Shutdown() {}  // 基类虚函数
};

class Derived : public Base {  // 派生类（子类），继承自 Base
public:
    void Shutdown() override {  // 重写基类的方法
        // 函数实现
    }
};
```

可继承的情况：

```cpp
// 示例1：普通类（可继承，非抽象）
class A {};

// 示例2：含虚函数的类（可继承，非抽象）
class B {
public:
    virtual void f() {}  // 普通虚函数
};

// 示例3：抽象类（必须被继承）
class C {
public:
    virtual void f() = 0;  // 纯虚函数：不含方法体
};

// 示例4：禁止继承的类
class D final {};  // 无法被继承
```

继承类重写：

```cpp
class Shape {
public:
    virtual void draw() { std::cout << "Shape" << std::endl; }
};

class Circle : public Shape {
public:
    void draw() override { std::cout << "Circle" << std::endl; }  // 重写
};

void render(Shape& shape) {
    shape.draw();  // 运行时根据实际类型调用对应方法
}

int main() {
    Circle c;
    render(c);  // 输出"Circle"
}
```

重载：

```cpp
class Math {
public:
    int add(int a, int b) { return a + b; }           // 重载版本1
    double add(double a, double b) { return a + b; }  // 重载版本2
    int add(int a, int b, int c) { return a + b + c; }  // 重载版本3
};

int main() {
    Math m;
    m.add(1, 2);      // 调用版本1
    m.add(1.5, 2.5);  // 调用版本2
    m.add(1, 2, 3);   // 调用版本3
}
```



引用传递1、值传递2：

```cpp
// 函数1：通过引用修改原变量
void add(int& a) {
    a += 1;  // 直接修改原变量
}

// 函数2：返回新值但不修改原变量
int add(int a) {
    return a + 1;  // 返回计算结果，原变量不变
}

// 调用示例
int main() {
    int x = 10;
    
    // 调用引用版本
    add(x);  // x 变为 11
    std::cout << x << std::endl;  // 输出 11
    
    // 调用值传递版本
    int y = add(x);  // x 仍为 11，y 是 12
    std::cout << x << ", " << y << std::endl;  // 输出 11, 12
}
```



**访问指针成员**：重点在于通过指针调用对象的成员函数，这是一种常规的对象方法调用操作。

```cpp
ptr->member  // 通过指针访问成员
(*ptr).member  // 等价写法：先解引用指针，再通过.访问成员
```

**回调函数**：是将函数作为参数传递给另一个函数，目的是在特定的时机执行。

```cpp
// 假设 AddExtender 的声明如下：
void AddExtender(const char* name, std::function<void()> callback);

// 调用时传递回调函数
GetFExtensibilityManager->AddExtender("MyExtender", []() {  // 匿名函数对象是实现回调的一种方式
    // 回调函数体
    std::cout << "Extender called!" << std::endl;
});
```

Lambda 表达式：

- **`[]`（捕获列表）**：指定如何捕获外部变量（如 `x`, `&y`）。
- **`()`（参数列表）**：类似普通函数的参数。
- **`-> return_type`（返回类型）**：可省略（编译器自动推导）。
- **`{}`（函数体）**：函数的具体实现。

```cpp
// 完整语法：
[capture](parameters) -> return_type { body }
// 简化形式
[]() { /* 函数体 */ }  // 完整形式
[] { /* 函数体 */ }    // 省略参数列表（常用）
```



直接调用1、回调函数2：

```cpp
// 厨师（函数）
std::string cookPasta() {
    return "🍝";
}

// 1. 顾客直接调用
std::string meal = cookPasta(); // 立即获得意大利面

// 餐厅（高阶函数）：接受一个回调函数
void orderFood(std::function<void(std::string)> callback) {
    // 做菜需要时间（异步操作）
    std::string meal = cookPasta();
    
    // 菜做好后，调用顾客的回调
    callback(meal); // 送餐
}

// 2. 顾客的回调函数（联系方式）
void customerCallback(std::string meal) {
    std::cout << "收到外卖：" << meal << std::endl;
}

// 顾客下单
orderFood(customerCallback); // 下单后继续做其他事，不阻塞
```



C++模板：

```cpp
// 函数模板：编译时实例化
template <typename T>  // typename 的作用是声明一个模板类型参数，告诉编译器 T 是一个类型（而不是一个值或其他东西）。
T max(T a, T b) {
    return a > b ? a : b;
}

// 使用
int result = max(3, 5);  // 编译器生成针对int的max函数
```

Java 泛型：

```java
// 泛型类：运行时擦除
public class Box<T> {
    private T value;
    public T get() { return value; }
}

// 使用
Box<String> box = new Box<>();  // 编译期检查类型，运行时类型擦除为Object
```

模板和泛型目的都是：让代码能够处理“不确定的数据类型”，避免为每种类型重复编写逻辑相同的代码。



**C++ 的 `struct`** ≈ **Java 的 `class`**（但成员默认 `public`）。  

```cpp
struct Point {  // C++ 结构体
    int x;
    int y;
};
```

```java
class Point {  // Java 类
    public int x;
    public int y;
}
```

**C++ 的 `std::vector`** ≈ **Java 的 `ArrayList`**（动态数组）。  

```cpp
std::vector<int> nums;  // C++ 容器
```

```java
ArrayList<Integer> nums = new ArrayList<>();  // Java 容器
```









