# DIY Maimai 触摸屏

本项目是对[Sucareto ↗](https://github.com/Sucareto/Mai2Touch)分享的项目的修改（我们基于的版本是由[HelloworldDk ↗](https://github.com/HelloworldDk/dkmaiproj/blob/main/newmap-llvermtn202212271340/newmap-llvermtn202212271340.ino)提供的），旨在提高触摸屏的稳定性和可用性。

## 任何从 newmap-llvermtn202212271340.ino 迁移过来的人，请阅读迁移指南**

## 入门指南

在你开始之前，我们强烈建议你阅读[原始仓库 ↗](https://github.com/HelloworldDk/dkmaiproj)的设置指南，以理解这个项目的基础知识。

## 项目改进

对原项目的改进如下：

1. **隔离了MPR121参数**：我们已经隔离了关键的MPR121参数以便于修改。这将允许用户自定义触摸屏的灵敏度和响应速度，以满足他们的特定需求。

2. **利用了 .touched() 方法**：我们已经从手动计算原始数据的阈值，转为使用 .touched() 方法。这不仅提高了触摸屏的速度，而且通过利用MPR121的防抖功能，增加了其稳定性。

3. **默认启用充放电时间搜索**：MPR121 能够为每个电极搜索最佳的充电和放电时间。出于某种原因，dk 的版本禁用了这个功能。我们发现启用这个功能可以使触摸屏更稳定。

## 迁移指南

### 从 newmap-llvermtn202212271340.ino 迁移

1. 用户应在 config.h 中定义阈值，而不是在主程序中。
2. touchmap 的第一列现在从0开始索引，而不是从1开始。用户应从 touchmap 的第一列中减去1。
3. touchmap 的第三列现在是偏移值，而不是实际的阈值。用户应先将所有的值改为0，然后调整偏移量。偏移量的可接受范围是-128到127。
4. 用户现在可以直接在 `config.h` 中定义每个mpr121的电极数量。用户不应更改 ino 文件中的代码。

### 从 Mai2TouchOK 或其他版本迁移

请首先阅读从 newmap-llvermtn202212271340.ino 的迁移指南，然后阅读代码中的注释，决定如何进行迁移。
