# SenType

Bộ gõ tiếng Việt Telex/VNI, một engine dùng chung cho macOS, Windows và Ubuntu (Fcitx5). Gõ trong
**preedit** của ứng dụng (gạch chân), chỉ commit khi gặp dấu cách / dấu câu / Enter / mất focus.
**Không** dùng backspace + gõ lại, nên thanh địa chỉ trình duyệt không bị autocomplete chen vào, Enter
vẫn là Enter (điều hướng, không tìm Google chuỗi phím thô). Ctrl/Alt/Cmd + phím: commit rồi để phím đi tiếp.

```
SenType/
  core/                    C++20 thuần, không phụ thuộc nền tảng
    include/sentype/       engine.hpp (Engine), session.hpp (chính sách phím dùng chung mac/win)
    src/, tests/           engine-test (80+ ca), session-test
  frontends/
    linux-fcitx5/          addon Fcitx5 + .deb + test tích hợp
    macos/                 InputMethodKit -> SenType.app (universal, ký ad-hoc)
    windows/               TSF text service -> sentype-tsf.dll (x64 + x86)
```

Build một frontend: `cmake -S . -B build -DSENTYPE_FRONTEND=<none|linux-fcitx5|macos|windows>`.
`none` chỉ build + test core: `cmake -S . -B build && cmake --build build && ctest --test-dir build`.
CI (`.github/workflows/build.yml` ở gốc repo) build cả ba nền tảng và chạy test.

## macOS (11+; build cần Command Line Tools)

```sh
cmake -S . -B build -DSENTYPE_FRONTEND=macos && cmake --build build
ctest --test-dir build
frontends/macos/install.sh build/frontends/macos/SenType.app   # chép vào ~/Library/Input Methods
```

Đăng xuất/đăng nhập, rồi System Settings -> Keyboard -> Input Sources -> Edit -> `+` -> Vietnamese ->
**SenType**. Menu của nguồn nhập (biểu tượng bàn phím trên menu bar) chọn Telex/VNI và kiểu đặt dấu.
Bản `build/SenType-macos.zip` ký ad-hoc: máy khác cần `xattr -dr com.apple.quarantine SenType.app`.
Chưa có icon (cần ảnh raster do bạn cung cấp).

## Windows (10/11)

Build bằng MSVC (CI) hoặc mingw-w64: `cmake -S . -B build -DSENTYPE_FRONTEND=windows`. Đặt
`x64\sentype-tsf.dll` và `x86\sentype-tsf.dll` cạnh `install.bat`, chạy **Run as administrator**
(`regsvr32` cả hai bản; app 32-bit dùng bản x86). Rồi Settings -> Time & language -> Language & region ->
Vietnamese -> Language options -> Add a keyboard -> **SenType**. Gỡ: `uninstall.bat`.
Chọn kiểu gõ: registry `HKCU\Software\SenType`, `Method`=`Telex`|`VNI`, `TonePlacement`=`Modern`|`Classic`.

## Linux (Ubuntu 26.04, Fcitx5)

### Cài đặt

```sh
sudo apt install ./fcitx5-sentype_0.1.0_amd64.deb
im-config -n fcitx5          # đặt Fcitx5 làm bộ gõ mặc định, rồi đăng xuất/đăng nhập
```

Thêm SenType: chạy `fcitx5-configtool` (gói `fcitx5-config-qt`) -> Input Method -> bỏ "Only Show Current
Language" -> chọn **SenType** -> mũi tên phải để thêm. Tuỳ chọn (Telex/VNI, kiểu đặt dấu `hoà`/`hòa`,
kiểm tra chính tả) nằm ở Addons -> SenType (hoặc nút Configure cạnh SenType).

### Wayland (GNOME, Ubuntu 26.04)

- Đảm bảo Fcitx5 tự chạy khi đăng nhập: gói `fcitx5` đã cài file autostart; nếu chưa chạy thì
  `fcitx5 -d` hoặc bật trong GNOME Tweaks -> Startup Applications.
- GNOME/Mutter nói chuyện trực tiếp qua giao thức `text-input-v3`; thường **không cần** đặt
  `GTK_IM_MODULE`/`QT_IM_MODULE`. Chỉ khi app Qt/GTK cũ không gõ được mới thêm vào
  `/etc/environment`: `GTK_IM_MODULE=fcitx`, `QT_IM_MODULE=fcitx`, `XMODIFIERS=@im=fcitx`.
- Chrome/Chromium chạy Wayland native: `--ozone-platform=wayland --enable-wayland-ime`
  (thêm `--wayland-text-input-version=3` nếu bản Chrome còn dùng v1). Bản Flatpak/Snap: cấp thêm quyền
  chạy Wayland. Firefox và GTK4 dùng text-input-v3 sẵn.

### Dùng

- Telex: `aa aw ow oo ee dd uw w`, dấu `s f r x j`, `z` xoá dấu. Gõ lặp phím dấu để huỷ (`ass` -> `as`).
- VNI: `6` mũ, `7` móc, `8` trăng, `9` đ, `1..5` dấu, `0` xoá dấu.
- Dấu gõ tự do (`toans` -> `toán`). Backspace xoá từng chữ trong từ. `Esc` trả lại đúng phím đã gõ.
- Từ không hợp lệ (`text`, `window`) được trả về nguyên bản khi commit (bật "SpellCheck").
- Ctrl/Alt/Super + phím: commit từ rồi để phím đi tiếp tới ứng dụng.

### Kiểm tra tay (lỗi URL bar)

1. Mở Chrome (Wayland + IME flags ở trên), bấm vào thanh địa chỉ, chọn SenType (Ctrl+Space).
2. Gõ `vieejt nam` -> thấy `việt nam` gạch chân, không có phần bôi sáng autocomplete.
3. Gõ thêm `.com` rồi Enter -> trình duyệt **đi tới** địa chỉ, không mở tìm kiếm Google.
4. Gõ `google.com` / `text` rồi Enter -> không bị đổi thành `gôgle.com`, `tẽt` (từ không hợp lệ được khôi phục).
5. Gõ `vieejt`, chuyển cửa sổ khác rồi quay lại -> từ đã được commit đúng một lần (không lặp).
6. Gõ `hoaf`, Backspace -> `hò`; `Esc` trên `tooi` -> `tooi`.

## Build Linux

Chạy trong Ubuntu 26.04 (cần `build-essential cmake ninja-build libfcitx5core-dev libfcitx5utils-dev
libfcitx5config-dev fcitx5-modules-dev file`):

```sh
cmake -S . -B build -G Ninja -DSENTYPE_FRONTEND=linux-fcitx5 && cmake --build build
ctest --test-dir build --output-on-failure   # core + nạp addon vào Fcitx5 thật
cd build && cpack -G DEB --config CPackConfig.cmake   # fcitx5-sentype_<ver>_amd64.deb
```
