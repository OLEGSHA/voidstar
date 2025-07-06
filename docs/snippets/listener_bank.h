template <int N> struct listener_slot {
  void (*the_listener)(textbox *) = nullptr;
  textbox *the_textbox = nullptr;
};

// Contains N slots through recursion by base class
// TODO: thread safety
template <int N> struct listener_bank : listener_bank<N - 1> {
  // Global variables for Nth slot
  static listener_slot<N> slot;

  // The function passed as callback - unique for each N
  static void callback_entrypoint() {
    auto &slt = listener_bank<N>::slot;
    slt.the_listener(slt.the_textbox);
  }

  // Find a free slot and set its globals
  static auto bind(void (*lst)(textbox *), textbox *txtbx) -> void (*)() {
    // Check slots 0..N-1 recursively first
    if (auto res = listener_bank<N - 1>::bind(lst, txtbx)) {
      return res;
    }

    auto &slt = listener_bank<N>::slot;
    if (slt.the_listener != nullptr) {
      return nullptr;
    }
    slt.the_listener = lst;
    slt.the_textbox = txtbx;
    return &callback_entrypoint;
  }
};

// Stop recursion
template <> struct listener_bank<-1> {
  static auto bind(...) -> void (*)() { return nullptr; }
  static bool unbind(...) { return false; }
};

// Up to 64 listeners at once
using listeners = listener_bank<64>;

void listener_a(textbox *txtbx) { get_text(txtbx); }

void listener_b(textbox *txtbx) { set_text(txtbx, "No!"); }

void setup(textbox *txtbx_a, textbox *txtbx_b) {
  // TODO: cleanup (unbind after callback is no longer used)
  add_listener(txtbx_a, listeners::bind(&listener_a, txtbx_a));
  add_listener(txtbx_b, listeners::bind(&listener_b, txtbx_b));
}
