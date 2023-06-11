#include <fstream>

#include "slide_window.h"

int main() {
    int i = 0;
    Slide_Window& sw = Slide_Window::get_instance();
    sw.init();
    std::cout << "test begin" << std::endl;

    while (i < 10000) {
        if (i % 100 == 0) {
            // 漏包
            i++;
            continue;
        }
        sw.add_data(i << 12);
        if (i % 133 == 0) {
            // 重复
            sw.add_data(i << 12);
        }
        if (i % 177 == 0) {
            // 乱序
            sw.add_data((i + 2) << 12);
            sw.add_data((i) << 12);
            sw.add_data((i + 1) << 12);
        }
        i++;
    }
    int error = 0;
    int index = 0;
    for (auto& data : sw.get_seq()) {
        index++;
        if (!data.got && (index % 100 != 0)) {
            error++;
            sw.display(index - 2, index + 2);
        }
    }
    std::cout << "error:" << error << std::endl;
}