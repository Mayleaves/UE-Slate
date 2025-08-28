# 菜单界面

`+ Select a Scene`：将 `Sequence` 加入 Job 队列（替换）。

`-`：删除 Job 队列中的 `Sequence` 。

`Job Info`：当前 Job 队列中的 `Sequence` 信息。

`Labeling Options`：当前 `CineCamera` 能够标注的最大距离。

`File Directory`：源/保存目录。

`Camera`：单选生成数据的 `CineCamera`。

`Image Label`：多选需要标注的 `StaticMesh`。

⚙/ `Label Management`：标注 `StaticMesh` 的类别名。

`Start Frame`：起始显示帧。

`End Frame`：结束显示帧。

`Play Range`：遍历显示帧范围。

`Start Labeling`：开始标注。

<img src="https://gitee.com/Koletis/pic-go/raw/master/202508281118416.png" alt="image-20250828111837076" style="zoom:80%;" />

单选（`CineCamera`）、多选（`StaticMesh`）、类别名（⚙）：

<img src="https://gitee.com/Koletis/pic-go/raw/master/202508281558868.png" alt="image-20250828155802810" style="zoom:80%;" /><img src="https://gitee.com/Koletis/pic-go/raw/master/202508281558164.png" alt="image-20250828155817089" style="zoom:80%;" /><img src="https://gitee.com/Koletis/pic-go/raw/master/202508281534835.png" alt="image-20250828153413728" style="zoom:80%;" />

# 数据生成与标注

<img width="1914" height="1018" alt="S9 WDC CA%Y~6)XOL6%B{PO" src="https://github.com/user-attachments/assets/50d55d79-4ee2-42a6-a6ed-4976e64d730a" />

<img src="https://gitee.com/Koletis/pic-go/raw/master/202508281133637.png" alt="image-20250828113335578" style="zoom:80%;" />

<img width="1722" height="723" alt="image" src="https://github.com/user-attachments/assets/d5743aa2-5af5-41d0-9b8c-607c60bcb127" />

# 代码说明

<img src="https://gitee.com/Koletis/pic-go/raw/master/202508281429111.png" alt="image-20250828142924854" style="zoom:80%;" />

`SyntheticDataFactory`：弹窗（父模块）

<img src="https://gitee.com/Koletis/pic-go/raw/master/202508281539020.png" alt="image-20250828153929947" style="zoom:80%;" />

`InteractButtons`：界面（子模块）

<img src="https://gitee.com/Koletis/pic-go/raw/master/202508281543860.png" alt="image-20250828151232312" style="zoom:80%;" />

`CommonTypes`：公共结构体模块

<img src="https://gitee.com/Koletis/pic-go/raw/master/202508281553981.png" alt="image-20250828143332466" style="zoom:80%;" />

`LevelSequenceUtils`：标注逻辑

<img src="https://gitee.com/Koletis/pic-go/raw/master/202508281514673.png" alt="image-20250828151426602" style="zoom:80%;" />

`OpenCVForUnreal`：画框

<img src="https://gitee.com/Koletis/pic-go/raw/master/202508281554122.png" alt="image-20250828151541957" style="zoom:80%;" />



