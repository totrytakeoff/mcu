## Keil ARM Compiler 6 (AC6) 迁移与构建指引

本指引记录了将一个原本面向 AC5/老版 CMSIS 的 STM32F10x 工程在 Keil AC6 (armclang) 环境下成功编译的步骤与注意事项，方便后续项目复用。

---

### 适用场景
- 工程原本依赖老版 CMSIS（`core_cm3.h` 等宏针对 `__CC_ARM`/`__GNUC__`，缺少 AC6 宏适配）。
- 使用 Keil uVision（示例路径 `D:\devtools\KEIL`）并希望通过命令行方式构建。

---

### 症状（典型报错）
AC6 编译时在 `core_cm3.h` 报大量错误，例如：
- “use of undeclared identifier 'asm'”
- `__ISB(arg)` / `__DSB(arg)` / `__DMB(arg)` 形参未声明

这些通常源自 CMSIS 版本较旧，仅为 `__CC_ARM`（armcc5）或 `__GNUC__` 提供了定义，未针对 `__ARMCC_VERSION >= 6`（armclang/AC6）定义对应宏与内联指令。

---

### 修复步骤

#### 1) 调整工程 IncludePath，优先使用 Keil 自带 CMSIS（AC6 适配版）
在 `*.uvprojx` 中更新 C 编译器的 `IncludePath`，把 Keil 自带 CMSIS 放在本地 CMSIS 之前（确保优先包含 AC6 适配头）：
```338:345:RCB6406_6412之应用-IO应用_电机驱动/RCB6406_6412-DEMO.uvprojx
<VariousControls>
  <MiscControls></MiscControls>
  <Define>USE_STDPERIPH_DRIVER, STM32F10X_HD</Define>
  <Undefine></Undefine>
  <IncludePath>D:\devtools\KEIL\ARM\CMSIS\Include;.\CMSIS;.\FWlib\inc;.\USER;.\USER\inc</IncludePath>
</VariousControls>
```
注意：保留本地 `./CMSIS` 以继续使用工程自带的设备头（如 `stm32f10x.h`、启动文件等），但让 AC6 版通用 CMSIS 头优先。

#### 2) 为 AC6 增加宏与内联指令适配（如本地 CMSIS 不够新时）
如果工程仍然包含旧版 `core_cm3.h` 并被引用，需为 `__ARMCC_VERSION >= 6010050` 分支补充 `__ASM` / `__INLINE` 定义以及内联指令实现。

修改点一（添加 AC6 分支）：
```290:305:RCB6406_6412之应用-IO应用_电机驱动/CMSIS/core_cm3.h
#if defined ( __CC_ARM   )
  #define __ASM            __asm
  #define __INLINE         __inline

#elif defined ( __ICCARM__ )
  #define __ASM           __asm
  #define __INLINE        inline
  #define __NOP           __no_operation

#elif defined ( __ARMCC_VERSION ) && ( __ARMCC_VERSION >= 6010050 )
  #define __ASM            __asm
  #define __INLINE         __inline

#elif defined   (  __GNUC__  )
  #define __ASM            asm
  #define __INLINE         inline
#endif
```

修改点二（为 AC6 提供内联指令实现）：
```776:793:RCB6406_6412之应用-IO应用_电机驱动/CMSIS/core_cm3.h
#elif (defined (__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050))
static __INLINE void __NOP()                      { __ASM volatile ("nop"); }
static __INLINE void __enable_irq()               { __ASM volatile ("cpsie i"); }
static __INLINE void __disable_irq()              { __ASM volatile ("cpsid i"); }
static __INLINE void __enable_fault_irq()         { __ASM volatile ("cpsie f"); }
static __INLINE void __disable_fault_irq()        { __ASM volatile ("cpsid f"); }
static __INLINE void __WFI()                      { __ASM volatile ("wfi");   }
static __INLINE void __WFE()                      { __ASM volatile ("wfe");   }
static __INLINE void __SEV()                      { __ASM volatile ("sev");   }
static __INLINE void __ISB()                      { __ASM volatile ("isb");   }
static __INLINE void __DSB()                      { __ASM volatile ("dsb");   }
static __INLINE void __DMB()                      { __ASM volatile ("dmb");   }
static __INLINE void __CLREX()                    { __ASM volatile ("clrex"); }
```

说明：
- 旧版 `__ISB(arg)/__DSB(arg)/__DMB(arg)` 形式对 AC6 不友好，改为无参形式即可。
- 如果你的工程使用了更新的 CMSIS 版本，通常已自带 AC6 兼容实现，可跳过此步。

---

### 命令行构建（PowerShell）
在 PowerShell 中使用调用运算符 `&` 执行 Keil uVision 命令行：
```bash
& "D:\devtools\KEIL\UV4\UV4.exe" -r "C:\Users\myself\Desktop\stm32_motor\RCB6406_6412之应用-IO应用_电机驱动\RCB6406_6412-DEMO.uvprojx" -t "Target 1" -j0 -o "C:\Users\myself\Desktop\stm32_motor\build.log"
```
- `-r`：Rebuild
- `-t`：Target 名称（示例为 “Target 1”）
- `-j0`：并行 Job 数（0 表示自动）
- `-o`：将构建输出写入日志文件

---

### 结果与验证
- 日志结尾应类似：
```1:35:build.log
".\Objects\RCB6441-DEMO.axf" - 0 Error(s), 0 Warning(s).
Build Time Elapsed:  00:00:01
```
- 生成产物：`Objects/RCB6441-DEMO.axf`、对应的 `.hex`（如已启用 FromELF）。

---

### 复用建议
- 优先通过 `uvprojx` 的 `IncludePath` 让 AC6 版 CMSIS 生效，尽量避免直接修改第三方文件；若必须修改，请集中记录差异，便于升级时对比。
- 若多个工程共享，可把上述 AC6 适配的 `core_cm3.h` 变更点整理成独立补丁。
- 若使用 STM32 标准外设库（StdPeriph），保留 `USE_STDPERIPH_DRIVER` 与对应系列宏（如 `STM32F10X_HD`），并确认启动文件、链接脚本与目标设备一致。

---

以上步骤已在 `STM32F103RC` 的示例工程中验证通过，可作为其他 STM32F1/F4 等老工程迁移到 Keil AC6 的参考。