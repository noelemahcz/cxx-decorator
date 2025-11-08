class deco1 final {
public:
  deco1(bool (*f)(int)) noexcept {
    // callbacks.add(f);
  }
};


class deco2 final {
public:
  deco2(bool (*f)(int)) noexcept {
    // callbacks.add(f);
  }
};


[[decorator(deco1) ,nodiscard]]
bool foo(int x);

[[nodiscard, decorator(deco1)]]
bool foo(int x) {
  return true;
}void baz() {}

[[nodiscard,decorator(deco1),maybe_unused]]bool foo(int x);

[[     decorator(deco1)        ]]bool foo(int x);


[[decorator(deco2)]]
bool bar(int x);

[[decorator(deco2)]]bool bar(int x);

