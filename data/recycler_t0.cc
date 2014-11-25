#define MORE_ENABLE_LOGGING 1
#include <more/gen/recycler.h>

struct C {
    void set_mark() { mark = true; }
    void unset_mark() { mark = false; }
    bool is_marked() const { return mark; }

  private:
    bool mark;
    bool xing;
};

const int N = 100;
C* array[N];

void mark_odd() {
    for (int i = 1; i < N; i += 2)
	if (array[i]) array[i]->set_mark();
}
void mark_even() {
    for (int i = 0; i < N; i += 2)
	if (array[i]) array[i]->set_mark();
}

int main() {
    more::gen::opt_recycler_do_log = true;

    for (int i = 0; i < N; ++i) array[i] = 0;

    more::gen::recycler_group g_rcyc;
    more::gen::recycler<C> rcyc(g_rcyc);

    for (int i = 0; i < N; ++i) {
	array[i] = rcyc.allocate(1);
	rcyc.construct(array[i]);
// 	std::clog << (void*)array[i] << '\n';
// 	std::clog.flush();
    }

    std::clog << "Adding odd marker.\n";
    g_rcyc.register_marker(mark_odd);

    for (int i = 0; i < N; ++i) {
	array[i] = rcyc.allocate(1);
	rcyc.construct(array[i]);
// 	std::clog << (void*)array[i] << '\n';
// 	std::clog.flush();
    }

    std::clog << "Adding even marker.\n";
    g_rcyc.register_marker(mark_even);

    for (int i = 0; i < N; ++i) {
	array[i] = rcyc.allocate(1);
	rcyc.construct(array[i]);
// 	std::clog << (void*)array[i] << '\n';
// 	std::clog.flush();
    }

}
