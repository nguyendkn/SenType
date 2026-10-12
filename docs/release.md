# Phát hành (release)

Đẩy tag `v*` thì `.github/workflows/release.yml` build 3 bộ cài, tính `SHA256SUMS` và tạo GitHub Release.

## Cắt release

```sh
git push                       # commit phải đã có trên remote
scripts/tag.sh --dry-run       # chỉ in tag sẽ tạo
scripts/tag.sh                 # tạo tag chú thích + push (REMOTE=origin để bỏ qua bước chọn remote)
```

Script từ chối khi: cây làm việc có thay đổi chưa commit (file tracked), HEAD chưa có trên remote, HEAD đã
mang tag release, fetch remote lỗi, không có remote. Nó không có `--force`: lỗi thì sửa nguyên nhân.

**Tag** = `v<VERSION>.<YYYYMMDD><INDEX>`, ví dụ `v0.1.202610120` (INDEX: 0 cho tag đầu tiên trong ngày,
rồi 1, 2, ...; tag trần `v0.1.20261012` coi như index 0). `VERSION` (`MAJOR.MINOR`, mặc định `0.1`) là
nguồn sự thật duy nhất: sửa file `VERSION` rồi commit để đổi bản chính. Workflow từ chối tag không khớp
định dạng hoặc không khớp file `VERSION`. CMake nhận `-DSENTYPE_VERSION=<tag bỏ chữ v>`.

| Nơi dùng | Giá trị cho tag `v0.1.202610120` |
|---|---|
| deb | `Version: 0.1.202610120-1` |
| macOS | `CFBundleShortVersionString`/pkg version `0.1.202610120` |
| Windows | `DisplayVersion` `0.1.202610120`; file version `0.1.0.0` (4 số 16-bit, không chứa nổi ngày) |

Chạy thử không publish: Actions -> release -> *Run workflow* (version dùng ngày UTC + index 0).

## Tệp trong release

| Tệp | Nền tảng |
|---|---|
| `fcitx5-sentype_<ver>-1_ubuntu26.04_amd64.deb`, `..._arm64.deb` | Ubuntu 26.04 |
| `SenType-<ver>.pkg` | macOS 11+ (universal arm64+x86_64) |
| `SenType-<ver>-macos.zip` | macOS, dự phòng: `SenType.app` + `install.sh`/`uninstall.sh` (cài theo user) |
| `SenType-<ver>-setup.exe` | Windows 10/11 (NSIS, x64 + x86) |
| `SHA256SUMS` | checksum mọi tệp trên |

Kiểm tra: tải tất cả vào một thư mục, `sha256sum -c SHA256SUMS` (macOS: `shasum -a 256 -c SHA256SUMS`;
Windows: `Get-FileHash <tệp>` rồi so với dòng tương ứng).

## Cài và gỡ

- **Ubuntu**: `sudo apt install ./fcitx5-sentype_*_amd64.deb` (apt kéo `fcitx5`); `postinst` in hướng dẫn
  (`im-config -n fcitx5`, `fcitx5-configtool`). Gỡ: `sudo apt remove fcitx5-sentype`.
- **macOS**: mở `.pkg`, cài vào `/Library/Input Methods/SenType.app` (cần quyền admin; postinstall tắt
  tiến trình SenType cũ và xoá bản cài theo user cũ nếu có). Đăng xuất/đăng nhập, rồi thêm SenType ở
  System Settings -> Keyboard -> Input Sources. Gỡ: `sudo ./uninstall.sh` (lấy từ file zip; xoá app,
  receipt `com.sentype.inputmethod.pkg`). Dự phòng không cần admin: giải nén zip, `./install.sh SenType.app`.
- **Windows**: chạy `SenType-<ver>-setup.exe` (xin quyền admin; im lặng: `/S`). DLL vào
  `C:\Program Files\SenType\x64` và `\x86`, `regsvr32` cả hai. Cài đè bản cũ được hỗ trợ (huỷ đăng ký
  bản cũ, DLL đang bị khoá thì đổi tên `.old` và xoá khi khởi động lại; đăng xuất/đăng nhập để ứng dụng
  nạp bản mới). Gỡ: Settings -> Apps -> SenType, hoặc `uninstall.exe /S`. Cài đặt người dùng
  (`HKCU\Software\SenType`) được giữ lại.

## Ký số, notarize

- **Mặc định không ký**: `.pkg` chưa ký + app ký ad-hoc, nên Gatekeeper chặn lần mở đầu (System Settings ->
  Privacy & Security -> *Open Anyway*, hoặc `xattr -d com.apple.quarantine <pkg>`). `setup.exe` chưa ký nên
  SmartScreen cảnh báo. Gói deb không ký.
- **macOS có Developer ID**: thêm secrets `MACOS_CERTS_P12` (base64 .p12 chứa cả *Developer ID Application* và
  *Installer*), `MACOS_CERTS_PASSWORD`, `MACOS_APP_IDENTITY`, `MACOS_PKG_IDENTITY` thì app được ký lại
  (hardened runtime) và pkg được ký. Thêm `NOTARY_APPLE_ID`, `NOTARY_TEAM_ID`, `NOTARY_PASSWORD`
  (app-specific) thì notarize + staple. Thiếu secret thì bước tương ứng tự bỏ qua. Đường ký/notarize
  **chưa chạy thử** (chưa có chứng chỉ).
- Windows: chưa có bước ký Authenticode.

## Runner Ubuntu

`.deb` build trong container `ubuntu:26.04`; amd64 chạy trên `ubuntu-24.04`, arm64 trên `ubuntu-24.04-arm`
(native). Chọn runner arm native thay vì QEMU vì fuzz test và test tích hợp Fcitx5 chậm hơn nhiều dưới
giả lập. Runner arm miễn phí cho repo public; repo private cần runner arm trả phí.

## Kiểm thử cục bộ

`tools/verify/deb.sh <ver> <out>` (trong image `tools/docker/linux-dev.Dockerfile`),
`frontends/macos/packaging/build-pkg.sh`, `tools/verify/windows-installer.sh <ver>` (image
`windows-dev.Dockerfile`). Wine trên Apple Silicon không chạy được installer NSIS (stub 32-bit);
`tools/verify/windows-installer-smoke.ps1` chạy cài/cài đè/gỡ thật trên runner Windows của CI.
