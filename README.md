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

<img width="700" height="458" src="https://gitee.com/Koletis/pic-go/raw/master/202508281118416.png" alt="image-20250828111837076"/>

单选（`CineCamera`）、多选（`StaticMesh`）、类别名（⚙）：

<img width="164" height="118" alt="image" src="https://github.com/user-attachments/assets/ff24d3d7-e566-412e-9c35-8856aff32c3d" />
<img width="131" height="294" alt="image" src="https://github.com/user-attachments/assets/523291a2-93a0-4ddb-9076-512409566108" />
<img width="475" height="413" alt="image" src="https://github.com/user-attachments/assets/7987cfe4-ad85-4855-8bb0-730b3ea5119f" />

# 数据生成与标注

<img width="1914" height="1018" alt="S9 WDC CA%Y~6)XOL6%B{PO" src="https://github.com/user-attachments/assets/50d55d79-4ee2-42a6-a6ed-4976e64d730a" />

<img width="700" height="458" alt="image" src="https://github.com/user-attachments/assets/30629d72-e732-479f-8510-f967d0ccbcc4" />

<img width="1722" height="723" alt="image" src="https://github.com/user-attachments/assets/d5743aa2-5af5-41d0-9b8c-607c60bcb127" />

# 代码说明

<img width="300" height="700" src="https://gitee.com/Koletis/pic-go/raw/master/202508281429111.png" alt="image-20250828142924854"/>

`SyntheticDataFactory`：弹窗（父模块）

<img width="300" height="400" src="https://gitee.com/Koletis/pic-go/raw/master/202508281539020.png" alt="image-20250828153929947"/>

`InteractButtons`：界面（子模块）

<img width="300" height="600" src="https://gitee.com/Koletis/pic-go/raw/master/202508281543860.png" alt="image-20250828151232312"/>

`CommonTypes`：公共结构体模块

<img width="200" height="300" src="https://gitee.com/Koletis/pic-go/raw/master/202508281553981.png" alt="image-20250828143332466"/>

`LevelSequenceUtils`：标注逻辑

<img width="250" height="350" src="https://gitee.com/Koletis/pic-go/raw/master/202508281514673.png" alt="image-20250828151426602"/>

`OpenCVForUnreal`：画框

<img width="200" height="300" src="https://gitee.com/Koletis/pic-go/raw/master/202508281554122.png" alt="image-20250828151541957"/>



