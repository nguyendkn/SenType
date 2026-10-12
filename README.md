# SenType

Bộ gõ tiếng Việt Telex/VNI, một lõi dùng chung cho macOS, Windows và Linux (Fcitx5). Gõ trong
**preedit** của ứng dụng (gạch chân), chỉ commit khi gặp dấu cách / dấu câu / Enter / mất focus.
**Không** dùng backspace + gõ lại: thanh địa chỉ trình duyệt không bị autocomplete chen vào, Enter vẫn
là Enter. Ctrl/Alt/Cmd + phím: commit rồi để phím đi tiếp.

```
core/                  C++20 thuần: Engine (gõ dấu), Session (chính sách phím), settings
frontends/             linux-fcitx5 · macos (IMKit) · windows (TSF): adapter mỏng
tools/                 fetch-test-data.sh, diff/ (so sánh UniKey/Bamboo), docker/
docs/                  architecture.md · testing.md · install.md · release.md
scripts/               tag.sh (tạo tag release), ci-import-macos-certs.sh
VERSION                MAJOR.MINOR, nguồn sự thật cho số phiên bản
```

Thiết kế: [docs/architecture.md](docs/architecture.md). Kiểm thử (unit, corpus âm tiết, fuzz, so sánh
UniKey/Bamboo, nguồn và giấy phép dữ liệu): [docs/testing.md](docs/testing.md). Cài đặt và lỗi đã biết
theo từng nền tảng: [docs/install.md](docs/install.md). Phát hành (tag, bộ cài deb/pkg/exe, checksum, ký số):
[docs/release.md](docs/release.md).

## Dùng

- Telex: `aa aw ow oo ee dd uw w`, dấu `s f r x j`, `z` xoá dấu. Gõ lặp phím dấu để huỷ (`ass` hiện `as`).
- VNI: `6` mũ, `7` móc, `8` trăng, `9` đ, `1..5` dấu, `0` xoá dấu.
- Dấu gõ tự do (`toans` -> `toán`, `nguoiwf` -> `người`). Backspace xoá từng chữ trong từ.
  `Esc` trả lại đúng phím đã gõ.
- Từ không phải âm tiết tiếng Việt (`text`, `window`, `class`) được trả lại **nguyên mọi phím đã nhấn**
  khi commit (tuỳ chọn SpellCheck, mặc định bật).
- Kiểu đặt dấu: `hoà` (Modern, mặc định) hoặc `hòa` (Classic).

## Build và test nhanh

```sh
sh tools/fetch-test-data.sh                       # tải corpus test (không commit vào repo)
cmake -S . -B build && cmake --build build && ctest --test-dir build --output-on-failure
cmake -S . -B build -DSENTYPE_FRONTEND=<linux-fcitx5|macos|windows>   # kèm một frontend
```

CI: `.github/workflows/build.yml` (core trên 3 hệ điều hành, fuzz libFuzzer+sanitizer, Linux .deb,
macOS bundle, Windows DLL x64/x86; so sánh UniKey/Bamboo chạy tay bằng `workflow_dispatch`). Release: `.github/workflows/release.yml`, chạy khi đẩy tag `v*` (`scripts/tag.sh`).

## Giấy phép

Chưa có file LICENSE: chủ repo cần chọn giấy phép (xem docs/testing.md về giấy phép dữ liệu test).
