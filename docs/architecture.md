# Kiến trúc SenType

Một lõi C++20 thuần (không phụ thuộc nền tảng) + ba frontend mỏng. Mọi chính sách xử lý phím nằm
trong lõi; frontend chỉ dịch sự kiện gốc sang `sentype::Key` và hiển thị kết quả.

```
 ┌──────────────┐   ┌──────────────┐   ┌──────────────┐
 │ Fcitx5 addon │   │ macOS IMKit  │   │ Windows TSF  │   frontend: chỉ làm 3 việc
 │ (Linux)      │   │ (SenType.app)│   │ (sentype-tsf)│   1. sự kiện gốc -> Key
 └──────┬───────┘   └──────┬───────┘   └──────┬───────┘   2. Key -> Session::on_key
        │ make_key()       │ make_key()       │ make_key() 3. KeyResult -> preedit/commit
        └──────────────────┼──────────────────┘
                           ▼
        ┌─────────────────────────────────────────────┐
        │ core/Session  (session.hpp)                 │  chính sách phím dùng chung:
        │  on_key / flush / reset / set_config        │  - ký tự in được -> vào từ
        │  would_handle (TSF OnTestKeyDown)           │  - Space, dấu câu, Enter, Tab, mũi tên:
        │  make_key (sự kiện gốc -> Key)              │    commit rồi để phím đi tiếp
        └───────────────────┬─────────────────────────┘  - Ctrl/Alt/Cmd/Win + phím: commit, nhả phím
                            ▼                            - Backspace: sửa từ đang gõ, rỗng thì nhả
        ┌─────────────────────────────────────────────┐  - Esc: trả lại đúng phím đã gõ
        │ core/Engine  (engine.hpp)                   │  - mất focus/deactivate: flush()
        │  feed / backspace / preedit / commit_text   │
        │   ├ WordState   (word-state*.cpp): chữ + dấu│
        │   ├ syllable-layout: âm đầu/vần/âm cuối, vị trí dấu thanh
        │   └ syllable-validator: kiểm tra âm tiết (khôi phục phím nếu sai)
        └─────────────────────────────────────────────┘
        settings.hpp: parse_method / parse_tone_style (tên cấu hình chung)
```

## Quy ước

- **Preedit-only**: gõ trong vùng gạch chân của ứng dụng, commit khi gặp ranh giới. Không dùng
  backspace + gõ lại, không phát phím giả. Nhờ vậy thanh địa chỉ trình duyệt không chen autocomplete
  và Enter vẫn là Enter.
- **Khôi phục phím**: từ không phải âm tiết tiếng Việt (bật SpellCheck) được trả lại đúng *mọi phím đã
  nhấn* (`class` -> `class`, không phải `clas`). Sau Backspace, từ được coi là do người dùng chỉnh nên
  không khôi phục.
- `Key::Kind::Char` chỉ gồm ASCII in được (0x20..0x7E); ký tự khác là `Other` (kết thúc từ).

## Frontend

| Nền tảng | Thư mục | Việc riêng của adapter |
|---|---|---|
| Linux Fcitx5 | `frontends/linux-fcitx5/addon` | `KeyEvent` -> `Key`; `setClientPreedit` + `commitString`; cấu hình qua `fcitx5-configtool` |
| macOS | `frontends/macos/src` | `NSEvent` -> `Key`; `setMarkedText` / `insertText`; menu nguồn nhập, `NSUserDefaults` |
| Windows | `frontends/windows/src` | `ITfKeyEventSink` -> `Key`; `ITfComposition`; registry `HKCU\Software\SenType` |

Thứ tự áp dụng `KeyResult`: macOS/Windows chèn `commit` trước rồi cập nhật composition; Fcitx xoá
preedit trước rồi `commitString` (Fcitx cần preedit rỗng trước khi commit). `Session` không bao giờ trả
đồng thời `commit` và `preedit` khác rỗng nên thứ tự này không ảnh hưởng kết quả.

## Cấu trúc thư mục

```
core/include/sentype/   engine.hpp  session.hpp  settings.hpp   API công khai
core/src/               cài đặt (word-state, syllable-*, letter, engine, session, settings)
core/tests/             engine-test  session-test  syllable-corpus-test  fuzz/  support/  data/
frontends/*             adapter mỏng + test dịch phím
tools/                  fetch-test-data.sh  diff/ (so sánh UniKey/Bamboo)  docker/
```
