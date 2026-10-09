# BTL_LTMM Rules

PHẦN 1: QUY TẮC LÀM VIỆC 


1. Không nộp bài thẳng vào thư mục chính (Nhánh main)

    Thư mục chính (nhánh main) là nơi chứa bản code/báo cáo hoàn hảo nhất. Không ai được quyền sửa trực tiếp vào đây.

    Cách làm đúng: Khi bắt đầu làm việc, mỗi người phải tự "copy" ra một bản nháp riêng (gọi là tạo Branch / Nhánh mới). Làm xong xuôi trên bản nháp của mình mới tính tiếp.

2. Làm đến đâu, Commit đến đó

    Quy tắc: Viết xong 1 hàm, tạo xong 1 file test, hay viết xong phần 1 của báo cáo -> Phải Commit ngay với lời nhắn rõ ràng (VD: "Đã viết xong hàm mã hóa HDB3", "Sửa lỗi sai chính tả phần mở đầu"). Không gộp chung làm 1 tuần rồi mới Commit 1 cục siêu to.

3. Tạo Đơn Xin Nộp Bài (Pull Request - PR)

    Sau khi làm xong trên "bản nháp" của mình, không được tự ý quăng nó vào thư mục chính.

    Bạn phải tạo một cái đơn gọi là Pull Request (PR) trên trang web GitHub. PR có nghĩa là: "Các bạn ơi, tớ làm xong phần việc của tớ rồi, xin phép được gộp nó vào bài chung của cả nhóm".

4. Code Review

    Khi có 1 bạn tạo Đơn xin nộp bài (PR), phải có ít nhất 1-2 người khác trong nhóm vào đọc thử.

    Nếu thấy sai: Comment bảo bạn ấy sửa.

    Nếu thấy đúng: Bấm nút "Approve" (Duyệt). Chỉ khi nào có đủ người Duyệt thì bài đó mới được chính thức gộp vào thư mục chính.

PHẦN 2: CÁCH ĐÓNG GÓP BÀI TẬP LỚN

1. Điểm Chăm chỉ - Số lần Commit: Máy sẽ đếm số lần bạn "Lưu báo cáo/code". Mỗi người cần ít nhất 5-10 commits trong suốt dự án. (Lưu ý: Cố tình thêm 1 dấu phẩy rồi commit để gian lận số lượng sẽ bị trừ điểm).

2. Điểm Sản phẩm - Nộp PR : Bạn phải là tác giả của ít nhất 1-2 "Đơn xin nộp bài" được gộp thành công vào thư mục chung. Nghĩa là bạn thực sự tạo ra giá trị (code thuật toán, tạo file test .txt, viết báo cáo file word/latex).

3. Đi Review dạo: Ai cũng phải vào đọc bài của người khác. Đọc, góp ý và bấm "Approve" (Duyệt) cho bài của thành viên khác ít nhất 2 lần.

4. Lưu ý cho các bạn không code thuật toán : Đừng lo nếu bạn không giỏi C/C++. Những bạn nhận nhiệm vụ viết Báo cáo, vẽ sơ đồ, hoặc tạo file Test input/output... chỉ cần soạn thảo file đó, đẩy lên GitHub (Commit và PR) y hệt như người viết code.

