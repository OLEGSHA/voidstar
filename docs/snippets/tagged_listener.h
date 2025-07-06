// Use a tag to create a unique template instance for each use of bind()
// TODO: thread safety
template <std::source_location const &tag> struct tagged_listener {
  // Global variables for this use of bind()
  static void (*the_listener)(textbox *) = nullptr;
  static textbox *the_textbox = nullptr;

  // The function passed as callback
  static void callback_entrypoint() { the_listener(the_textbox); }
};

// At compile time: create new global variables. At runtime: set them.
template <std::source_location const &tag = std::source_location::current()>
auto bind(void (*lst)(textbox *), textbox *txtbx) -> void (*)() {
  using globals = tagged_listener<tag>;
  globals::the_listener = lst;
  globals::the_textbox = txtbx;
  return &globals::callback_entrypoint;
}

void listener_a(textbox *txtbx) { get_text(txtbx); }

void listener_b(textbox *txtbx) { set_text(txtbx, "No!"); }

void setup(textbox *txtbx_a, textbox *txtbx_b) {
  // TODO: cleanup (unbind after callback is no longer used)
  add_listener(txtbx_a, bind(&listener_a, txtbx_a)); // bind<sloc-1>(...)
  add_listener(txtbx_b, bind(&listener_b, txtbx_b)); // bind<sloc-2>(...)
}
