# Cài đặt và lỗi đã biết

Cách dùng chung: gõ Telex/VNI, từ hiện gạch chân (preedit), Space/dấu câu/Enter/mất focus thì commit.
Chọn Telex/VNI và kiểu đặt dấu (`hoà` hay `hòa`) theo từng nền tảng bên dưới.

## Linux (Ubuntu 26.04, Fcitx5)

```sh
sudo apt install ./fcitx5-sentype_0.1.0_amd64.deb
im-config -n fcitx5          # đặt Fcitx5 làm bộ gõ mặc định, rồi đăng xuất/đăng nhập
```

Build .deb: xem README (`cpack -G DEB`). Thêm SenType: `fcitx5-configtool` (gói `fcitx5-config-qt`) ->
Input Method -> bỏ "Only Show Current Language" -> **SenType** -> mũi tên phải. Tuỳ chọn (Telex/VNI,
kiểu đặt dấu, SpellCheck) ở Addons -> SenType.

Chọn engine khi chạy: Ctrl+Space (đổi bộ gõ của Fcitx5).

### Wayland / GNOME

- Fcitx5 phải chạy khi đăng nhập (gói `fcitx5` có file autostart); không thì `fcitx5 -d`.
- GNOME nói chuyện qua `text-input-v3`, thường không cần đặt `GTK_IM_MODULE`/`QT_IM_MODULE`. Chỉ khi
  ứng dụng Qt/GTK cũ không gõ được mới thêm vào `/etc/environment`: `GTK_IM_MODULE=fcitx`,
  `QT_IM_MODULE=fcitx`, `XMODIFIERS=@im=fcitx`.
- **Chưa kiểm chứng trên GNOME**: các cờ Chrome Wayland (`--ozone-platform=wayland
  --enable-wayland-ime`, `--wayland-text-input-version=3`) là hướng dẫn chung, chưa thử trên GNOME thật.

### Chrome trên Wayland (quan sát trên sway, KHÔNG phải GNOME)

Môi trường: Ubuntu 26.04 x86_64, sway headless, Chrome 155.

- Chrome chạy Wayland native **bị crash mỗi khi Fcitx5 đang chạy**, kể cả khi chỉ có bàn phím
  `keyboard-us` (không có SenType). Vậy lỗi không riêng của SenType; chưa rõ nguyên nhân gốc.
- Cách chạy được: qua XWayland + GTK IM:

  ```sh
  GTK_IM_MODULE=fcitx google-chrome --ozone-platform=x11 --gtk-version=4
  ```

- Chưa kiểm tra trên GNOME/Mutter, KDE hay Chrome bản khác; đừng suy ra rằng GNOME cũng bị.

### Kết quả E2E đã xác minh

Ubuntu 26.04 x86_64, sway + XWayland + xdotool, Chrome (cấu hình XWayland ở trên), SenType Telex: gõ
vào thanh địa chỉ `Vieejt nguwowif dduowcj` ra **`Việt người được`**; autocomplete nội tuyến của
omnibox + Enter vẫn **điều hướng tới URL** (không tìm Google chuỗi phím thô).
Chỉ thử với Chrome trên cấu hình này; các ứng dụng khác chưa thử E2E.

### Lỗi đã biết (Linux)

- Chrome Wayland native + Fcitx5 crash trên sway (xem trên).
- Test tích hợp dùng Fcitx5 thật nhưng với `testfrontend`, không có màn hình: không thay cho thử tay
  trong ứng dụng thật.
- GNOME chưa thử.

## macOS (11+)

```sh
cmake -S . -B build -DSENTYPE_FRONTEND=macos && cmake --build build
frontends/macos/install.sh build/frontends/macos/SenType.app   # chép vào ~/Library/Input Methods
```

Đăng xuất/đăng nhập, rồi System Settings -> Keyboard -> Input Sources -> Edit -> `+` -> Vietnamese ->
**SenType**. Menu nguồn nhập (biểu tượng bàn phím trên menu bar) chọn Telex/VNI, Classic.
Bản `build/SenType-macos.zip` ký ad-hoc: máy khác cần `xattr -dr com.apple.quarantine SenType.app`.

Lỗi/giới hạn: chưa có icon (cần ảnh raster do chủ repo cung cấp); chỉ kiểm tra build, `codesign
--verify` và test dịch phím, **chưa** thử gõ thật trong ứng dụng ở lần kiểm tra gần nhất; không có
Apple Developer ID nên không notarize.

## Windows (10/11)

Build MSVC (CI) hoặc mingw-w64: `cmake -S . -B build -DSENTYPE_FRONTEND=windows`. Đặt `x64\sentype-tsf.dll`
và `x86\sentype-tsf.dll` cạnh `install.bat`, chạy **Run as administrator** (`regsvr32` cả hai bản). Rồi
Settings -> Time & language -> Language & region -> Vietnamese -> Language options -> Add a keyboard ->
**SenType**. Gỡ: `uninstall.bat`. Kiểu gõ: registry `HKCU\Software\SenType`, `Method` = `Telex`|`VNI`,
`TonePlacement` = `Modern`|`Classic` (không phân biệt hoa/thường).

Lỗi/giới hạn: chỉ kiểm tra biên dịch chéo mingw + `regsvr32` dưới wine; **chưa** thử trên Windows thật
(TSF composition, ứng dụng UWP/elevated). DLL chưa ký số nên SmartScreen có thể cảnh báo.
