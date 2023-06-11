#ifndef __SLIDE_WINDOWN_H__
#define __SLIDE_WINDOWN_H__

#include <unistd.h>

#include <iostream>
#include <vector>

struct Data {
    __uint16_t data;
    __uint8_t seq_num;
    bool got;
    Data(__uint8_t seq_num) : data(0), seq_num(seq_num), got(false) {}
    Data(__uint16_t data, __uint8_t seq_num, bool got)
        : data(data), seq_num(seq_num), got(got) {}
    Data(Data&& _x) : data(_x.data), seq_num(_x.seq_num), got(_x.got) {}
    Data& operator=(Data& _x) {
        data = _x.data;
        seq_num = _x.seq_num;
        got = _x.got;
        return *this;
    }
};

typedef std::vector<Data> Sequence_t;

class Slide_Window final {
   public:
    void init(size_t _window_max_len = 8);
    ~Slide_Window();
    static Slide_Window& get_instance() {
        if (__instance == nullptr) {
            __instance = new Slide_Window();
        }
        return *__instance;
    }
    void add_data(__uint16_t input);
    Sequence_t& get_seq();
    __int64_t get_error_cnt() const { return error_cnt; }
    void display(size_t begin, size_t end) const;

   private:
    Slide_Window();
    inline static Slide_Window* __instance;
    size_t window_max_len;
    __int64_t error_cnt = 0;
    size_t window_begin = 0;
    const size_t seq_num_period = 16;
    Sequence_t sequence;
};

#endif