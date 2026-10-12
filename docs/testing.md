# Hướng dẫn kiểm thử

Mọi test dùng dữ liệu thật, không mock. Dữ liệu test **không** nằm trong sản phẩm và không vendor vào repo.

| Lớp | Lệnh | Nội dung |
|---|---|---|
| Unit (core) | `ctest` -> `core-engine`, `core-session` | 86 ca bảng (Telex/VNI/dấu/Backspace/Esc), chính sách phím `Session`, `make_key`, cài đặt |
| Corpus âm tiết | `core-syllable-corpus` | 6.590 âm tiết thật x Telex/VNI x nhiều thứ tự gõ |
| Fuzz/property | `core-fuzz-random`, `core-fuzz-random-corpus`, `core-fuzz-smoke` | bất biến trên chuỗi phím ngẫu nhiên |
| Frontend | `macos-key-translation`, `windows-key-translation`, `addon-integration` | dịch phím gốc, nạp addon vào Fcitx5 thật |
| So sánh UniKey/Bamboo | `tools/diff/run-diff.sh` (chạy tay) | cùng chuỗi phím vào 3 engine |

## Chạy

```sh
sh tools/fetch-test-data.sh                  # tải corpus vào build/test-data (cần mạng, perl, curl)
cmake -S . -B build && cmake --build build
ctest --test-dir build --output-on-failure   # thiếu corpus: test corpus/fuzz-corpus bị SKIP (mã 77), không báo xanh giả
```

Sanitizer + libFuzzer (clang; macOS Apple clang không có libFuzzer, dùng Docker):

```sh
docker build -f tools/docker/linux-dev.Dockerfile -t sentype-dev tools/docker
docker run --rm -v "$PWD":/src -w /src sentype-dev bash -c '
  sh tools/fetch-test-data.sh && cmake -S . -B /tmp/b -G Ninja -DCMAKE_CXX_COMPILER=clang++ \
    -DSENTYPE_SANITIZE=ON -DSENTYPE_FUZZ=ON && cmake --build /tmp/b && ctest --test-dir /tmp/b --output-on-failure
  /tmp/b/core/sentype-fuzz -max_total_time=600 -max_len=96 core/tests/fuzz/corpus'   # fuzz dài
cmake -S . -B build-asan -DSENTYPE_SANITIZE=ON   # ASan+UBSan native (Apple clang cũng được)
```

So sánh UniKey/Bamboo (Docker, sinh `build/diff/*.txt` và bảng tổng kết):

```sh
docker run --rm -v "$PWD":/src sentype-dev sh /src/tools/diff/run-diff.sh 20000 1
```

## Corpus âm tiết (Task A)

- **Nguồn**: `vi_VN.dic` của hunspell-vi (LibreOffice/dictionaries, `vi/`, commit ghim trong
  `tools/fetch-test-data.sh`, kiểm sha256). Gốc: từ điển của GS Hồ Ngọc Đức, bộ kiểm chính tả của Ivan
  Garcia và Nguyễn Xuân Minh. Độc lập với validator của SenType.
- **Giấy phép**: dữ liệu `vi_VN` là **GPL-2** (xem `LICENSES-en.txt` của nguồn). Vì vậy script chỉ **tải**
  về `build/test-data` (đã .gitignore); không commit, không đóng gói vào sản phẩm. Từ tiếng Anh (phần
  negatives) lấy từ `en/en_US.dic` cùng repo; cũng chỉ tải, không commit. File repo commit chỉ gồm danh sách
  do chúng tôi viết: `core/tests/data/non-native-syllables.txt` (15 mục bị loại, kèm lý do) và
  `english-must-restore.txt` (70 từ). Giấy phép dữ liệu `en_US` chưa được rà lại kỹ: không commit nên không
  ảnh hưởng repo, nhưng chủ repo nên tự xác nhận trước khi tái phân phối.
- **Lọc**: 6.631 dòng -> 6.605 mục chữ thường toàn chữ Việt; bỏ 15 mục không phải âm tiết bản ngữ (đa âm tiết
  `email internet intranet palăng tivi basoi`; chữ vay/viết tắt `gen gip gram ka têt tout v web xit`) ->
  **6.590 âm tiết chuẩn**. Từ điển dùng đặt dấu kiểu mới (`hoà`) nhất quán: 69 âm tiết khác kiểu cũ,
  0 âm tiết theo kiểu cũ.
- **Kiểm** (mỗi âm tiết): (1) `is_valid_syllable`; (2) gõ ra đúng NFC bằng Telex và VNI theo 4 thứ tự
  (dấu ở cuối, dấu ngay sau vần, dấu mũ/móc gõ sau, dấu thanh trước dấu mũ/móc), cộng chữ Hoa đầu và
  IN HOA toàn bộ; (3) từ tiếng Anh/kỹ thuật phải trả lại nguyên phím.
- **Lưu ý trung thực**: quy tắc glide (`oa/oe/uy` theo âm đầu), `oo`, `oao`, tập âm cuối của `y/oe`, `uơ`
  được chỉnh **sau khi thấy dữ liệu này** nên 100% không phải số đo trên tập giữ riêng. Kiểm chứng độc lập
  khác là so sánh với UniKey/Bamboo trên cùng 6.590 âm tiết (bên dưới).
- Mục `[info] composed word in dictionary`: từ ngẫu nhiên/tiếng Anh mà engine vẫn ghép thành âm tiết hợp lệ
  nhưng từ điển (từ vựng, không phải ngữ âm) không liệt kê. Chỉ để tham khảo, không phải lỗi.

## Fuzz / property (Task B)

Cùng bộ bất biến (`core/tests/fuzz/fuzz-invariants.cpp`) cho libFuzzer và ctest:

1. không crash/UB (ASan + UBSan, `-fno-sanitize-recover`);
2. mọi chuỗi trả về là UTF-8 hợp lệ, NFC (chỉ ASCII hoặc chữ Việt dựng sẵn), không có dấu kết hợp lẻ (U+0300..036F);
3. phát lại xác định: hai engine cùng chuỗi phím -> cùng preedit/commit;
4. Backspace bỏ đúng một chữ; với phím không biến đổi, Backspace x k == gõ tiền tố;
5. `raw_text()` == các phím đã nhấn; Esc (`Session`) trả đúng chuỗi đó;
6. `commit_text`/`reset`/`flush` luật đẳng; commit là phím thô hoặc từ đã ghép cùng số chữ;
7. `Session`: preedit == preedit của engine, key không tiêu thụ thì không có preedit.

## So sánh UniKey và Bamboo (Task C)

`tools/diff/` chỉ dùng cho kiểm thử: build trong Docker, **không** link vào SenType, không vendor mã.
UniKey: `ukengine` của github.com/fcitx/fcitx5-unikey (LGPL-2.0+; harness mô phỏng luồng phím của
fcitx5-unikey: free marking bật, kiểu mới, spell check + tự khôi phục từ sai). Bamboo:
github.com/BambooEngine/bamboo-core (MIT; cờ free tone marking + autocorrect, kiểu mới; từ không hợp lệ
khôi phục phím thô như ibus/fcitx5-bamboo). SenType: Telex/VNI, Modern, SpellCheck bật.

Phân loại (`tools/diff/classify.py`, luật đầu tiên khớp thắng; chỉ gắn nhãn, không đổi kết quả):

- `reference-quirk/odd-one-out`: engine còn lại đồng ý với SenType (heuristic, hai engine cùng sai thì lọt).
- `reference-quirk/stricter-than-dictionary`: SenType ghép ra âm tiết có trong từ điển, tham chiếu trả phím thô.
- `style/vni-digit-ends-word`: VNI, chữ số không có đích ngắt từ (số vẫn là số); tham chiếu giữ chữ số trong từ.
- `style/dictionary-strict`: tham chiếu ghép thứ từ điển không có (`bêt`, `ka`, `ce`, `coa`...), SenType trả phím thô.
- `style/stroke-needs-dd`: SenType chỉ nhận `dd` ngay sau âm đầu (`did` không thành `đi`).
- `unclassified`: cần người xem.

Kết quả và danh sách lỗi đã sửa: xem mục "Kết quả" cuối file.

## Kết quả (lần chạy gần nhất)

Corpus: từ điển 6.605 mục -> loại 15 -> **6.590 âm tiết chuẩn**; 61.728 lượt gõ (Telex 17.541 + VNI 17.827 theo 4 thứ tự gõ,
kèm 26.360 lượt chữ Hoa đầu và IN HOA) -> **100%** đúng NFC; validator chấp nhận **6.590/6.590**; 140/140 lượt
từ tiếng Anh/kỹ thuật trả lại nguyên phím.

So sánh (55.625 chuỗi phím: 35.402 âm tiết x 8, 64 câu, 159 từ Anh, 20.000 gõ ngẫu nhiên; chạy trong Docker):

| Tham chiếu | giống hệt | style | reference-quirk | sentype-bug | unclassified |
|---|---:|---:|---:|---:|---:|
| Bamboo (bamboo-core) | 55.209 | 341 | 74 | 0 | 1 |
| UniKey (ukengine) | 54.974 | 341 | 309 | 0 | 1 |

`sentype-bug = 0` nghĩa là sau khi sửa không còn khác biệt nào được gán là lỗi SenType (không có luật tự gán
nhãn này: lỗi tìm ra được sửa rồi chạy lại). `unclassified = 1` là `epres` (SenType: `ếp`; cả hai tham chiếu
giữ phím thô; `ếp` hợp lệ về ngữ âm nhưng không có trong từ điển): chưa quyết là lỗi hay khác biệt.
Chi tiết theo luật: `style/vni-digit-ends-word` 315, `style/dictionary-strict` 24, `style/stroke-needs-dd` 2
(mỗi tham chiếu); phần quirk: `odd-one-out` (Bamboo 74, UniKey 309; UniKey từ chối `boóng`, `coóc`,
`khoào`, `gích`... và không khôi phục `boss`/`coffee`).

Lỗi lõi tìm được và đã sửa nhờ các test này (danh sách đầy đủ trong lịch sử git):

1. nhân `oo` (`boóng`, `soóc`) và `oao` (`ngoáo`, `khoào`) bị validator từ chối;
2. `uu`+`w` đặt móc sai chữ (`buuw` -> `buư`, phải là `bưu`);
3. phím `o` sau `oe`/`oa` biến thành mũ (`khoeo` -> `khôe`) thay vì là chữ thường;
4. vần chặn (`c ch p t`) được nhận khi không có dấu (`bêt`, `bôt`); nay phải có sắc/nặng;
5. nhân lướt `oa/oe/uy/uâ/uê` nhận mọi âm đầu (`boá`, `coả`); nay theo tập âm đầu quan sát được;
6. từ không hợp lệ chỉ trả lại phím sau khi huỷ dấu (`class` -> `clas`); nay trả đúng mọi phím đã nhấn;
7. Backspace sau `qu`+nguyên âm biến phím dấu thanh thành chữ (phát hiện bởi fuzz);
8. phím dấu khác loại không đổi được dấu đã gõ (`oow` -> `ơ`, `awa` -> `â`);
9. `uơ` nhận mọi âm đầu (`rươ`); nhân `y`/`oe` nhận âm cuối sai (`yc`, `oem`).

Khác biệt cố ý so với tham chiếu: `did` không thành `đi` (phải `dd`); chữ số VNI không có đích kết thúc từ;
SenType chặt hơn theo từ điển (`bôt`, `ka`, `ce`).

### Fuzz

- ctest (mọi OS): 2 x 200.000 chuỗi ngẫu nhiên có seed cố định (một nửa là gõ thật âm tiết từ điển + sửa) ~ 3-4
  triệu sự kiện phím mỗi lượt, có cả bản ASan+UBSan.
- libFuzzer (clang 21, ASan+UBSan, Docker linux arm64): 2.606.581 lượt chạy trong 604 s (4 worker song song) + 621.060 lượt (200 s); không crash, không vi phạm bất biến. Thêm ~3 triệu chuỗi ngẫu nhiên
  trên macOS (seed 2..7). Bug duy nhất do fuzz bắt được là mục 7 ở trên (bởi test ngẫu nhiên, trước khi chạy
  libFuzzer).
