#include "slide_window.h"

std::ostream& operator<<(std::ostream& out, const Data& data) {
    out << "(" << (int)data.seq_num << "," << (int)data.data << ","
        << (int)data.got << ")";
    return out;
}

Slide_Window::Slide_Window() {}

Slide_Window::~Slide_Window() {}

void Slide_Window::init(size_t _window_max_len) {
    window_max_len = _window_max_len;
}

// inline Slide_Window& Slide_Window::get_instance() {
//     if (__instance == nullptr) {
//         __instance = new Slide_Window();
//     }
//     return *__instance;
// }

// 可处理的错误：重复数据、漏几个数据、相近的几个数据乱序、
void Slide_Window::add_data(__uint16_t input) {
    Data data(input & 0xfff, input >> 12, true);
    if (sequence.size() > 0) {
        bool in_window = false;
        size_t diff =
            (data.seq_num + seq_num_period - sequence[window_begin].seq_num) %
            seq_num_period;
        if (diff + window_begin < sequence.size()) {
            // 窗口中有该序号，获取其迭代器
            Sequence_t::iterator prob_iter =
                sequence.begin() + diff + window_begin;
            if (prob_iter->got) {
                // 已被获取，且相等，为重复数据
                if (prob_iter->data == data.data) {
                    in_window = true;
                } else {
                    in_window = true;
                    // if (!sequence[window_begin].got) {
                    //     display_window();
                    // }
                }
            } else {
                // 未被获取，直接赋值
                *prob_iter = data;
                in_window = true;
            }
        }
        // 扩大窗口
        if (!in_window) {
            for (__uint8_t last = (sequence.back().seq_num + 1) & 0xf;
                 last != data.seq_num; last++, last &= 0xf) {
                sequence.push_back(Data(last));
                if (sequence.size() - window_begin >= window_max_len) {
                    if (!(sequence.begin() + window_begin++)->got) {
                        error_cnt++;
                    }
                }
            }
            sequence.push_back(std::move(data));
            if (sequence.size() - 1 - window_begin >= window_max_len) {
                if (!(sequence.begin() + window_begin++)->got) {
                    error_cnt++;
                }
            }
        }
    } else {
        sequence.push_back(std::move(data));
    }
}

Sequence_t& Slide_Window::get_seq() { return sequence; }

void Slide_Window::display(size_t begin, size_t end) const {
    for (int i = begin; i < sequence.size() && i < end; i++) {
        std::cout << sequence[i] << i << "# ";
    }
    std::cout << std::endl;
}
