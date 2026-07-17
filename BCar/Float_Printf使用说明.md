# Float_Printf 使用说明

- **日期**：2026-07-05
- **目的**：替代 `sprintf` 的 `%f` 格式化，节省约 5KB FLASH（去掉 `-u _printf_float`）

---

## 宏定义

```c
#define FLOAT_SIGN_OnlyNEGATIVE    0   /* 仅负数显示-，正数不显示符号 */
#define FLOAT_SIGN_ALWAYS          1   /* 始终显示 +/- */
#define FLOAT_SIGN_OnlyPOSITIVE    2   /* 仅正数显示+，负数不显示符号 */
```

---

## 函数签名

```c
char* Float_Printf(float value, int width, int decimals, uint8_t showSign);
```

| 参数 | 类型 | 说明 |
|------|------|------|
| `value` | float | 要格式化的浮点数 |
| `width` | int | 总显示宽度（含符号、整数部分、小数点、小数部分） |
| `decimals` | int | 小数位数（0~5） |
| `showSign` | uint8_t | `FLOAT_SIGN_OnlyNEGATIVE`=仅负数显示-，`FLOAT_SIGN_ALWAYS`=始终显示+/-，`FLOAT_SIGN_OnlyPOSITIVE`=仅正数显示+ |

**返回值**：格式化后的字符串指针（旋转缓冲区，无需手动释放）

---

## 调用示例

### OLED_Printf 使用

```c
// PID参数（不显示正负号）
OLED_Printf(0, 8, OLED_6X8, "P:%s", Float_Printf(AnglePID.Kp, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));
OLED_Printf(0, 16, OLED_6X8, "I:%s", Float_Printf(AnglePID.Ki, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));
OLED_Printf(0, 24, OLED_6X8, "D:%s", Float_Printf(AnglePID.Kd, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));

// Target/Angle（显示正负号）
OLED_Printf(0, 32, OLED_6X8, "T:%s", Float_Printf(AnglePID.Target, 5, 1, FLOAT_SIGN_ALWAYS));
OLED_Printf(0, 40, OLED_6X8, "A:%s", Float_Printf(Angle, 5, 1, FLOAT_SIGN_ALWAYS));

// Out（整数值，显示正负号，0位小数）
OLED_Printf(0, 48, OLED_6X8, "O:%s", Float_Printf(AnglePID.Out, 5, 0, FLOAT_SIGN_ALWAYS));

// OutOffset（固定值，显示正负号）
OLED_Printf(0, 56, OLED_6X8, "OOSP%s", Float_Printf(AnglePID.OutOffset_Positive, 5, 0, FLOAT_SIGN_ALWAYS));
OLED_Printf(58, 56, OLED_6X8, " OOSN%s", Float_Printf(AnglePID.OutOffset_Negative, 5, 0, FLOAT_SIGN_ALWAYS));

// AngleAcc_Offset（仅正数显示+，负数不显示符号）
OLED_Printf(0, 0, OLED_6X8, "AngleAcc_Offset:%s", Float_Printf(AngleAcc_Offset, 5, 2, FLOAT_SIGN_OnlyPOSITIVE));
```

### sprintf 使用（串口输出）

```c
char msg[150];
sprintf(msg, "[plot,%s,%s]",
    Float_Printf(SpeedPID.Actual, 5, 2, FLOAT_SIGN_OnlyNEGATIVE),
    Float_Printf(SpeedPID.Target, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));
HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
```

---

## 输出格式对照

| showSign | value | 输出 | 说明 |
|:--------:|-------|------|------|
| `FLOAT_SIGN_OnlyNEGATIVE` | 3.14 | `03.14` | 仅负数显示 -，正数不显示符号 |
| `FLOAT_SIGN_OnlyNEGATIVE` | -3.14 | `-03.14` | 仅负数显示 - |
| `FLOAT_SIGN_ALWAYS` | 3.1 | `+03.1` | 始终显示 +/- |
| `FLOAT_SIGN_ALWAYS` | -3.1 | `-03.1` | 始终显示 +/- |
| `FLOAT_SIGN_OnlyPOSITIVE` | 3.14 | `+03.14` | 仅正数显示 + |
| `FLOAT_SIGN_OnlyPOSITIVE` | -3.14 | `03.14` | 负数不显示符号 |
| `FLOAT_SIGN_OnlyNEGATIVE` | 0.5 | `00.50` | 整数部分补零 |
| `FLOAT_SIGN_ALWAYS` | 1000 | `+1000` | 0位小数，宽度不够原样显示 |
| `FLOAT_SIGN_ALWAYS` | -50 | `-0050` | 0位小数，宽度5 |
| `FLOAT_SIGN_OnlyNEGATIVE` | 2.49 | `02.49` | 宽度5，2位小数 |

---

## width 计算规则

```
总宽度 = 符号(0或1) + 整数位数 + 小数点(0或1) + 小数位数
```

当 `width > 总宽度` 时，在符号后、整数前补零。

示例：`value=3.14, width=5, decimals=2, showSign=FLOAT_SIGN_ALWAYS`
- 符号：1 位 (`+`)
- 整数：1 位 (`3`)
- 小数点：1 位 (`.`)
- 小数：2 位 (`14`)
- 总需：5 位，width=5，无需补零 → `+3.14`

示例：`value=3.14, width=6, decimals=2, showSign=FLOAT_SIGN_ALWAYS`
- 总需：5 位，width=6，补 1 个零 → `+03.14`

---

## 注意事项

1. **旋转缓冲区**：40 个缓冲区轮流使用，支持一次调用中使用多个 `Float_Printf`（最多 40 个）
2. **不四舍五入**：小数部分直接截断（`3.145` → `3.14`）
3. **宽度溢出**：数值本身超过 width 时原样显示（`123.45` + width=5 → `123.45`）
4. **RAM 开销**：40 × 16 = 640 字节

---

## 与原 sprintf 对照

```c
// 改前（需要 -u _printf_float，占 5KB FLASH）
sprintf(buf, "%05.2f", value);      // FLOAT_SIGN_OnlyNEGATIVE
sprintf(buf, "%+05.1f", value);     // FLOAT_SIGN_ALWAYS

// 改后（不依赖 _printf_float）
sprintf(buf, "%s", Float_Printf(value, 5, 2, FLOAT_SIGN_OnlyNEGATIVE));
sprintf(buf, "%s", Float_Printf(value, 5, 1, FLOAT_SIGN_ALWAYS));
sprintf(buf, "%s", Float_Printf(value, 5, 2, FLOAT_SIGN_OnlyPOSITIVE));
```
