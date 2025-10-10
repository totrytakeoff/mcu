# AT89S52 固件上传脚本 (USBASP)
# 适用于 TLE100 遥控器

Write-Host "=====================================" -ForegroundColor Cyan
Write-Host "   AT89S52 Firmware Uploader (USBASP)   " -ForegroundColor Cyan
Write-Host "=====================================" -ForegroundColor Cyan
Write-Host ""

# 检查固件文件
$firmware = ".pio\build\STC89C52\firmware.hex"
if (-not (Test-Path $firmware)) {
    Write-Host "❌ 错误: 固件文件不存在！" -ForegroundColor Red
    Write-Host "请先运行: pio run" -ForegroundColor Yellow
    exit 1
}

Write-Host "✅ 固件文件: $firmware" -ForegroundColor Green
Write-Host "✅ 芯片型号: AT89S52" -ForegroundColor Green
Write-Host "✅ 编程器: USBASP" -ForegroundColor Green
Write-Host ""

# 检查 avrdude 是否安装
$avrdude = Get-Command avrdude -ErrorAction SilentlyContinue
if (-not $avrdude) {
    Write-Host "❌ 错误: avrdude 未安装！" -ForegroundColor Red
    Write-Host ""
    Write-Host "安装方法:" -ForegroundColor Yellow
    Write-Host "  1. 下载 avrdude: https://github.com/avrdudes/avrdude/releases" -ForegroundColor White
    Write-Host "  2. 解压并添加到 PATH" -ForegroundColor White
    Write-Host "  或使用 winget: winget install avrdude" -ForegroundColor White
    exit 1
}

Write-Host "✅ avrdude 已安装: $($avrdude.Source)" -ForegroundColor Green
Write-Host ""

# 显示连接提示
Write-Host "⚠️  硬件连接检查:" -ForegroundColor Yellow
Write-Host "   USBASP → TLE100 ISP 接口（2×10 牛角座）" -ForegroundColor White
Write-Host "   Pin 1: MOSI → P1.5" -ForegroundColor White
Write-Host "   Pin 5: RST  → RST" -ForegroundColor White
Write-Host "   Pin 7: SCK  → P1.7" -ForegroundColor White
Write-Host "   Pin 9: MISO → P1.6" -ForegroundColor White
Write-Host "   Pin 2/10: VCC/GND（可选）" -ForegroundColor White
Write-Host ""
Write-Host "⚠️  注意:" -ForegroundColor Yellow
Write-Host "   1. 确保 USBASP 已连接到电脑" -ForegroundColor White
Write-Host "   2. 如果 USBASP 供电（Pin 2），遥控器不要另外上电" -ForegroundColor White
Write-Host "   3. 如果遥控器独立供电，USBASP Pin 2 悬空" -ForegroundColor White
Write-Host ""
Write-Host "按 Enter 继续..." -ForegroundColor Cyan
Read-Host

# 上传固件
Write-Host "🚀 开始上传..." -ForegroundColor Green
Write-Host ""

avrdude -c usbasp -p 89s52 -U flash:w:$firmware`:i

if ($LASTEXITCODE -eq 0) {
    Write-Host ""
    Write-Host "✅ 上传成功！" -ForegroundColor Green
    Write-Host ""
    Write-Host "📝 测试方法:" -ForegroundColor Cyan
    Write-Host "   1. 断开 USBASP" -ForegroundColor White
    Write-Host "   2. 连接 USB-TTL 到 TXD (P3.1)" -ForegroundColor White
    Write-Host "   3. 运行: pio device monitor -b 9600" -ForegroundColor White
    Write-Host "   4. 按下遥控器按键" -ForegroundColor White
    Write-Host "   5. 应该看到单个字符输出: F, B, L, R, U, D, W, X, Y, Z" -ForegroundColor White
} else {
    Write-Host ""
    Write-Host "❌ 上传失败！" -ForegroundColor Red
    Write-Host ""
    Write-Host "常见问题:" -ForegroundColor Yellow
    Write-Host "   1. USBASP 驱动未安装" -ForegroundColor White
    Write-Host "      解决: 安装 libusb 或 Zadig 驱动" -ForegroundColor Gray
    Write-Host "   2. USBASP 连接不良" -ForegroundColor White
    Write-Host "      解决: 检查排线连接，确保插紧" -ForegroundColor Gray
    Write-Host "   3. 目标芯片供电问题" -ForegroundColor White
    Write-Host "      解决: 确保遥控器上电或 USBASP 供电" -ForegroundColor Gray
    Write-Host "   4. 芯片被保护（熔丝位锁定）" -ForegroundColor White
    Write-Host "      解决: 可能需要高压编程器" -ForegroundColor Gray
    exit 1
}
