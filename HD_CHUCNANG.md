# HƯỚNG DẪN CHỨC NĂNG HỆ THỐNG MÁY ẤP TRỨNG THÔNG MINH

---

## 1. Tự Động Điều Chỉnh Thông Số Theo Từng Thời Điểm & Giai Đoạn Nở

Hệ thống được tích hợp sẵn thông số sinh học chuẩn của **6 loại trứng phổ biến**:
1. 🥚 **Trứng Gà** (21 ngày): Ấp 37.5°C - 60% | Cận nở (từ ngày 19) hạ 37.0°C - 70%
2. 🦆 **Trứng Vịt** (28 ngày): Ấp 37.8°C - 65% | Cận nở (từ ngày 25) hạ 37.1°C - 75%
3. 🐦 **Trứng Cút** (17 ngày): Ấp 37.5°C - 55% | Cận nở (từ ngày 15) hạ 37.0°C - 65%
4. 🕊️ **Trứng Bồ Câu** (18 ngày): Ấp 38.0°C - 58% | Cận nở (từ ngày 16) hạ 37.3°C - 68%
5. 🪿 **Trứng Ngỗng** (30 ngày): Ấp 37.7°C - 68% | Cận nở (từ ngày 27) hạ 37.0°C - 80%
6. 🦚 **Chim Trĩ** (24 ngày): Ấp 37.6°C - 60% | Cận nở (từ ngày 21) hạ 37.0°C - 70%
7. 🧪 **Chế Độ Test** (3 ngày): Ấp 27.0°C - 60% | Cận nở (từ ngày 2) lên 28.0°C - 65% (Dành riêng mô phỏng & thử nghiệm thiết bị)

### Nguyên Lý Tự Động Chuyển Giai Đoạn:
- **Giai đoạn Ấp phôi (Đầu chu kỳ):** Duy trì nhiệt độ ấm ổn định và độ ẩm vừa phải để phôi hình thành mạch máu và khung xương.
- **Giai đoạn Cận nở (3 ngày cuối):** Hệ thống **tự động hạ nhiệt độ** (~0.5°C) và **tự động nâng độ ẩm** (+10% đến 12%) để làm mềm vỏ trứng, giúp gà/vịt/chim non dễ mổ vỏ và đạp ra ngoài mà không bị khô dính sát vỏ.
- **Đồng hồ RTC DS1302:** Tự động tăng số ngày ấp khi qua nửa đêm (00:00).

---

## 2. Cơ Chế Tự Động Điều Tiết Rơ-le (Đèn Sưởi & Quạt Gió)

Hệ thống tự động điều khiển 2 rơ-le bám sát nhiệt độ và độ ẩm mục tiêu của từng giai đoạn:

1. **Điều khiển Đèn Sưởi:**
   - **Khi thiếu nhiệt ($T < Target - 0.2^\circ\text{C}$):** Đèn tự động **BẬT** để cấp nhiệt.
   - **Khi đạt đủ nhiệt ($T \ge Target$):** Đèn tự động **TẮT** ngay để tránh quán tính nhiệt làm lò bị nóng quá.
2. **Điều khiển Quạt Gió:**
   - **Khi quá nhiệt ($T \ge Target + 0.4^\circ\text{C}$):** Quạt tự động **BẬT** để thổi khí nóng ra ngoài, hạ nhiệt khẩn cấp.
   - **Khi ẩm quá cao ($H > Target + 5\%$):** Quạt tự động **BẬT** để thoát bớt hơi nước, chống ủng trứng.
   - **Khi về mức an toàn ($T \le Target$ và $H \le Target$):** Quạt tự động **TẮT** để giữ nhiệt độ ổn định.
3. **Cơ Chế Bảo Vệ Quá Nhiệt Khẩn Cấp (Cứu Phôi Trứng):**
   - Nếu nhiệt độ chạm mức nguy hiểm $\ge 38.3^\circ\text{C}$ (kể cả khi người dùng lỡ bấm bật đèn bằng tay trên Web), hệ thống sẽ **CƯỠNG BỨC TẮT ĐÈN NGAY LẬP TỨC và BẬT QUẠT TỐI ĐA** để chống chín trứng!
4. **Đảo Khí & Cung Cấp Oxy Định Kỳ:**
   - Nếu quạt không cần chạy trong 15 phút, hệ thống sẽ tự động bật quạt chạy nhẹ 20 giây để luân chuyển không khí, tránh tích tụ khí $CO_2$ và làm đều nhiệt độ khắp buồng ấp.

---

## 3. Hệ Thống Còi Báo Thông Minh (Smart Buzzer)

> **Mặc định:** Còi luôn **TẮT** trong quá trình máy hoạt động bình thường, tuyệt đối không gây ồn.

Còi chỉ phát âm thanh theo các quy ước riêng biệt sau:
1. **1 Tiếng "Bíp" ngắn (70ms):** Báo nhận lệnh khi bạn bấm nút vật lý hoặc bấm Bật/Tắt quạt/đèn/máy trên Web UI.
2. **2 Tiếng "Bíp - Bíp" dứt khoát:** Xác nhận **CẬP NHẬT CHUẨN THÀNH CÔNG** (khi Áp dụng hồ sơ trứng mới, Lưu thông số tùy chỉnh, hoặc Đổi ngày ấp trên Web/LCD).
3. **Tít dồn dập, liên tục (120ms kêu / 120ms ngắt):** 
   - **Cảnh báo QUÁ NHIỆT NGUY HIỂM** ($\ge 38.3^\circ\text{C}$ hoặc vượt ngưỡng mục tiêu 0.8°C).
   - Rơ-le quạt gió sẽ tự động bật cưỡng bức để tản nhiệt khẩn cấp, ngăn ngừa chết phôi.
4. **Bíp 5 nhịp đều (200ms kêu / 800ms ngắt, nghỉ 30s):**
   - **Cảnh báo QUÁ LẠNH** ($\le Target - 1.5^\circ\text{C}$).
   - Rơ-le đèn sưởi bật tối đa công suất để cấp nhiệt bù. Dành cho người nuôi kịp thời kiểm tra bóng sưởi hoặc nắp thùng hở.
5. **Tít kép ngắt quãng (Bíp-bíp rồi nghỉ 2s):**
   - **Cảnh báo ĐỘ ẨM NGUY CẤP** (Độ ẩm $< 45\%$ trong giai đoạn cận nở hoặc $> 88\%$).
   - Báo hiệu người dùng cần châm thêm nước vào khay tạo ẩm.

---

## 4. Can Thiệp & Tùy Chỉnh Trực Tiếp (2 Cách)

### Cách 1: Bằng Nút Bấm Vật Lý Trên Arduino (3 Nút)
- **Màn hình chính (Mode 0):** 
  - Dòng 1: Nhiệt độ & Độ ẩm thực tế (`T:37.5C  H:60%`).
  - Dòng 2: Mã loại trứng, Giai đoạn (`A`: Ấp, `N`: Nở), Trạng thái Đèn (`D:ON/OF`) & Quạt (`Q:ON/OF`).
- **Chọn hồ sơ nhanh (Mode 1):** Bấm **Nút 1** để chuyển Mode 1 $\rightarrow$ Bấm **Nút 1** chọn loại trứng $\rightarrow$ Bấm **Nút 2 (OK)** để lưu.
- **Tùy biến thông số chi tiết (Mode 2):** Bấm **Nút 3 (Edit)** $\rightarrow$ Dùng **Nút 1 (+)** / **Nút 2 (-)** để tăng giảm từng thông số (`tAp`, `hAp`, `tNo`, `hNo`, `days`, `hatchDay`).

### Cách 2: Qua Giao Diện Web UI (Điện Thoại / Máy Tính)
- **Đổi loại trứng:** Mở danh sách 6 loại trứng $\rightarrow$ Nhấn **"Áp dụng hồ sơ"**.
- **Chỉnh sửa nâng cao:** Nhấn nút **"⚙️ Chỉnh sửa"** để mở bảng nhập số liệu tùy ý $\rightarrow$ Nhấn **"Lưu & Kích hoạt"** (thông số sẽ truyền ngay lập tức sang Arduino).
- **Đổi ngày ấp nhanh:** Nhấn nút **"✏️ Đổi ngày ấp"** (tiện lợi khi mất điện hoặc chuyển trứng từ ổ khác sang).
- **Rơ-le thủ công:** Bấm trực tiếp nút **💡 ĐÈN SƯỞI** hoặc **🌀 QUẠT GIÓ** để bật/tắt tức thì.
- **Bật / Tắt máy:** Hai nút **▶️ Bắt đầu chu kỳ** và **⏸️ Tạm dừng máy**.

---

## 5. Giao Diện Web UI Trực Quan Mới
- **Thẻ hiển thị loại trứng đang ấp:** Biểu tượng sinh động (🥚, 🦆, 🐦, 🕊️, 🪿, 🦚), tên loại trứng và huy hiệu giai đoạn (`🌱 Ấp Phôi` hoặc `🐣 Cận Nở`).
- **Mục tiêu tự động thời gian thực:** Hiển thị rõ hệ thống đang nhắm tới nhiệt độ và độ ẩm bao nhiêu.
- **Thanh cảnh báo thông minh:** Tự động chớp đỏ và hiển thị chi tiết nguyên nhân sự cố nếu nhiệt độ hay độ ẩm vượt ngưỡng an toàn.
