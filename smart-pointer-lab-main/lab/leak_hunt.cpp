// =============================================================================
// RAII & Smart Pointers — Stage 5: The Leak Hunt
// leak_hunt.cpp
//
// This file contains THREE bugs. Your job:
//
//   Step 1 — Read the code and predict what will go wrong for each bug
//             BEFORE compiling. Write your predictions as comments.
//
//   Step 2 — Compile and run. Compare the cerr log against the
//             EXPECTED OUTPUT block at the bottom of this file.
//             Notice which "destroyed" lines are missing or wrong.
//
//   Step 3 — Compile with AddressSanitizer (see below). Each bug produces
//             a distinct ASan report — match each report to a bug.
//
//   Step 4 — Fix all three bugs using only RAII / smart-pointer tools.
//             No raw new/delete in your fix.
//
// Compile (buggy):
//   g++ -std=c++20 -Wall -Wextra -fsanitize=address,undefined -g leak_hunt.cpp -o leak_hunt
//
// The three bug types:
//   Bug A — raw new with no delete on an exception path
//   Bug B — shared_ptr reference cycle (same as Stage 4)
//   Bug C — unique_ptr passed by value when the caller doesn't need ownership
// =============================================================================

#include <iostream>
#include <memory>
#include <vector>
#include <stdexcept>
#include <atomic>

static int next_id() {
    static std::atomic<int> counter{0};
    return ++counter;
}

// ─────────────────────────────────────────────────────────────────────────────
// Resource: a simple named heap-allocated value that logs its lifetime.
// ─────────────────────────────────────────────────────────────────────────────
struct Resource {
    int         id_;
    std::string label_;
    int         value_;

    Resource(std::string label, int value)
        : id_(next_id()), label_(std::move(label)), value_(value)
    {
        std::cerr << "[Resource #" << id_ << " \"" << label_ << "\" val=" << value_ << "] born\n";
    }

    ~Resource() {
        std::cerr << "[Resource #" << id_ << " \"" << label_ << "\"] destroyed\n";
    }

    void describe() const {
        std::cerr << "  Resource #" << id_ << " \"" << label_ << "\" = " << value_ << "\n";
    }
};

// =============================================================================
// BUG A — raw new leaks on exception path
//
// process_data allocates a Resource with raw new, does some work,
// then deletes it. But if the work throws, the delete is skipped.
//
// Your prediction: since risky work can throw, process_data might not reach the delete statement, leading to a memory leak
// ASan symptom:    Direct leak of 48 byte(s) in 1 object(s) allocated from:
    // #0 0x7ff716c98548 in operator new(unsigned long) ../../../../src/libsanitizer/asan/asan_new_delete.cpp:95
    // #1 0x6373dfe5a927 in process_data(int) /workspaces/redesigned-octo-engine/smart-pointer-lab-main/lab/leak_hunt.cpp:80
    // #2 0x6373dfe5b7c1 in main /workspaces/redesigned-octo-engine/smart-pointer-lab-main/lab/leak_hunt.cpp:168
    // #3 0x7ff7160051c9 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
    // #4 0x7ff71600528a in __libc_start_main_impl ../csu/libc-start.c:360
    // #5 0x6373dfe5a5c4 in _start (/workspaces/redesigned-octo-engine/smart-pointer-lab-main/lab/leak_hunt+0x1c5c4) (BuildId: 7d31ffbe5a2c09aeeadd75d887450fa3b5b00b03)

// Fix:             Using unique_ptr instead of raw new to manage the Resource. This way, even if an exception is thrown, the unique_ptr's destructor will automatically clean up the Resource, preventing memory leaks.
// =============================================================================
static void risky_work(int x) {
    if (x > 50)
        throw std::runtime_error("value too large");
    std::cerr << "  risky_work: processed " << x << "\n";
}

void process_data(int x) {
    // Resource* r = new Resource("process_data", x);
    auto r = std::make_unique<Resource>("process_data", x);
    risky_work(x);

    r->describe();
    //  delete r; 
}

// =============================================================================
// BUG B — ???
//
// Your prediction: neither of Publisher and Subscriber objects will be destroyed due to a reference cycle created by shared_ptrs, leading to a memory leak
// ASan symptom:    Indirect leak of 40 byte(s) in 1 object(s) allocated from:
//     #0 0x79b347c23548 in operator new(unsigned long) ../../../../src/libsanitizer/asan/asan_new_delete.cpp:95
//     #1 0x5cbe02364c38 in std::__new_allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >::allocate(unsigned long, void const*) /usr/include/c++/13/bits/new_allocator.h:151
//     #2 0x5cbe02362fb3 in std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >::allocate(unsigned long) /usr/include/c++/13/bits/allocator.h:198
//     #3 0x5cbe02362fb3 in std::allocator_traits<std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > >::allocate(std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >&, unsigned long) /usr/include/c++/13/bits/alloc_traits.h:482
//     #4 0x5cbe02362fb3 in std::__allocated_ptr<std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > > std::__allocate_guarded<std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > >(std::allocator<std::_Sp_counted_ptr_inplace<Subscriber, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >&) /usr/include/c++/13/bits/allocated_ptr.h:98
//     #5 0x5cbe02361bae in std::__shared_count<(__gnu_cxx::_Lock_policy)2>::__shared_count<Subscriber, std::allocator<void>, int>(Subscriber*&, std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr_base.h:969
//     #6 0x5cbe02360e75 in std::__shared_ptr<Subscriber, (__gnu_cxx::_Lock_policy)2>::__shared_ptr<std::allocator<void>, int>(std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr_base.h:1712
//     #7 0x5cbe0235fac3 in std::shared_ptr<Subscriber>::shared_ptr<std::allocator<void>, int>(std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr.h:464
//     #8 0x5cbe0235ce63 in std::shared_ptr<Subscriber> std::make_shared<Subscriber, int>(int&&) /usr/include/c++/13/bits/shared_ptr.h:1010
//     #9 0x5cbe02356c5a in run_pubsub() /workspaces/redesigned-octo-engine/smart-pointer-lab-main/lab/leak_hunt.cpp:130
//     #10 0x5cbe02357750 in main /workspaces/redesigned-octo-engine/smart-pointer-lab-main/lab/leak_hunt.cpp:183
//     #11 0x79b346f901c9 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
//     #12 0x79b346f9028a in __libc_start_main_impl ../csu/libc-start.c:360
//     #13 0x5cbe023565c4 in _start (/workspaces/redesigned-octo-engine/smart-pointer-lab-main/lab/leak_hunt+0x1c5c4) (BuildId: 2e5423dd12672874bfd8c6929e52c2e668667c25)

// Indirect leak of 40 byte(s) in 1 object(s) allocated from:
//     #0 0x79b347c23548 in operator new(unsigned long) ../../../../src/libsanitizer/asan/asan_new_delete.cpp:95
//     #1 0x5cbe02364a26 in std::__new_allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >::allocate(unsigned long, void const*) /usr/include/c++/13/bits/new_allocator.h:151
//     #2 0x5cbe02362333 in std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >::allocate(unsigned long) /usr/include/c++/13/bits/allocator.h:198
//     #3 0x5cbe02362333 in std::allocator_traits<std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > >::allocate(std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >&, unsigned long) /usr/include/c++/13/bits/alloc_traits.h:482
//     #4 0x5cbe02362333 in std::__allocated_ptr<std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > > std::__allocate_guarded<std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> > >(std::allocator<std::_Sp_counted_ptr_inplace<Publisher, std::allocator<void>, (__gnu_cxx::_Lock_policy)2> >&) /usr/include/c++/13/bits/allocated_ptr.h:98
//     #5 0x5cbe0236170a in std::__shared_count<(__gnu_cxx::_Lock_policy)2>::__shared_count<Publisher, std::allocator<void>, int>(Publisher*&, std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr_base.h:969
//     #6 0x5cbe02360b29 in std::__shared_ptr<Publisher, (__gnu_cxx::_Lock_policy)2>::__shared_ptr<std::allocator<void>, int>(std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr_base.h:1712
//     #7 0x5cbe0235f911 in std::shared_ptr<Publisher>::shared_ptr<std::allocator<void>, int>(std::_Sp_alloc_shared_tag<std::allocator<void> >, int&&) /usr/include/c++/13/bits/shared_ptr.h:464
//     #8 0x5cbe0235cd1a in std::shared_ptr<Publisher> std::make_shared<Publisher, int>(int&&) /usr/include/c++/13/bits/shared_ptr.h:1010
//     #9 0x5cbe02356b95 in run_pubsub() /workspaces/redesigned-octo-engine/smart-pointer-lab-main/lab/leak_hunt.cpp:129
//     #10 0x5cbe02357750 in main /workspaces/redesigned-octo-engine/smart-pointer-lab-main/lab/leak_hunt.cpp:183
//     #11 0x79b346f901c9 in __libc_start_call_main ../sysdeps/nptl/libc_start_call_main.h:58
//     #12 0x79b346f9028a in __libc_start_main_impl ../csu/libc-start.c:360
//     #13 0x5cbe023565c4 in _start (/workspaces/redesigned-octo-engine/smart-pointer-lab-main/lab/leak_hunt+0x1c5c4) (BuildId: 2e5423dd12672874bfd8c6929e52c2e668667c25)

// SUMMARY: AddressSanitizer: 80 byte(s) leaked in 2 allocation(s).
// Fix:             make either of the pointers (Publisher's subscriber_ or Subscriber's publisher_) a weak_ptr
// =============================================================================
struct Subscriber;

struct Publisher {
    int id_;
    std::shared_ptr<Subscriber> subscriber_; 

    explicit Publisher(int id) : id_(id) {
        std::cerr << "[Publisher  #" << id_ << "] born\n";
    }
    ~Publisher() {
        std::cerr << "[Publisher  #" << id_ << "] destroyed\n";
    }
};

struct Subscriber {
    int id_;
    std::weak_ptr<Publisher> publisher_;

    explicit Subscriber(int id) : id_(id) {
        std::cerr << "[Subscriber #" << id_ << "] born\n";
    }
    ~Subscriber() {
        std::cerr << "[Subscriber #" << id_ << "] destroyed\n";
    }
};

void run_pubsub() {
    auto pub = std::make_shared<Publisher>(1);
    auto sub = std::make_shared<Subscriber>(2);

    pub->subscriber_ = sub;
    sub->publisher_  = pub;

    std::cerr << "  pub.use_count=" << pub.use_count()
              << "  sub.use_count=" << sub.use_count() << "\n";
}

// =============================================================================
// BUG C — unique_ptr passed by value unnecessarily
//
// Your prediction: Since unique_ptr is moved to print_resource, the unique_ptr in use_resource will be freed after print_resource returns, leading to a dangling pointer and potential use-after-free when we try to access r->value_ in use_resource after the call to print_resource
// Fix:             pass the unique_ptr by as a const raw pointer
//
// Note: ASan won't catch this one — it's not a memory error, it's a logic
// error. The log will show the resource dying at the wrong time.
// =============================================================================
void print_resource(const Resource* r) { 
    std::cerr << "  print_resource: ";
    r->describe();
}

void use_resource() {
    auto r = std::make_unique<Resource>("use_resource", 77);

    print_resource(r.get()); 

    // What's the value of r now?
    std::cerr << "  back in use_resource, value=" << r->value_ << "\n"; 
}

// =============================================================================
// main — runs all three buggy scenarios
// =============================================================================
int main() {
    std::cerr << "=== Stage 5: Leak Hunt ===\n\n";

    // ── Bug A ──────────────────────────────────────────────────────────────
    std::cerr << "-- Bug A: process_data(10) --\n";
    try { process_data(10); } catch (...) {}
    std::cerr << "  (no exception — did the resource get destroyed?)\n\n";

    std::cerr << "-- Bug A: process_data(99) --\n";
    try {
        process_data(99);
    } catch (const std::exception& e) {
        std::cerr << "  caught: " << e.what() << "\n";
    }
    std::cerr << "  (exception path — was the resource destroyed before the throw?)\n\n";

    // ── Bug B ──────────────────────────────────────────────────────────────
    std::cerr << "-- Bug B: pub/sub cycle --\n";
    run_pubsub();
    std::cerr << "  (did Publisher and Subscriber both get destroyed?)\n\n";

    // // ── Bug C ──────────────────────────────────────────────────────────────
    // std::cerr << "-- Bug C: unique_ptr by value --\n";
    // // Comment this out once you understand the crash — fix it first.
    // use_resource();
    // std::cerr << "  (was the resource still alive after print_resource returned?)\n\n";

    std::cerr << "=== end of main ===\n";
    return 0;
}

// =============================================================================
// EXPECTED OUTPUT after all three bugs are fixed
// (IDs may differ; the important thing is the pairing of born/destroyed)
//
// === Stage 5: Leak Hunt ===
//
// -- Bug A: process_data(10) --
// [Resource #1 "process_data" val=10] born
//   risky_work: processed 10
//   Resource #1 "process_data" = 10
// [Resource #1 "process_data"] destroyed        <-- must appear BEFORE "no exception"
//   (no exception — did the resource get destroyed?)
//
// -- Bug A: process_data(99) --
// [Resource #2 "process_data" val=99] born
// [Resource #2 "process_data"] destroyed        <-- must appear BEFORE "caught"
//   caught: value too large
//   (exception path — was the resource destroyed before the throw?)
//
// -- Bug B: pub/sub cycle --
// [Publisher  #1] born
// [Subscriber #2] born
//   pub.use_count=1  sub.use_count=1             <-- use_count stays 1 with weak_ptr fix
// [Subscriber #2] destroyed                     <-- both must appear here
// [Publisher  #1] destroyed
//   (did Publisher and Subscriber both get destroyed?)
//
// -- Bug C: unique_ptr by value --
// [Resource #3 "use_resource" val=77] born
//   print_resource:   Resource #3 "use_resource" = 77
//   back in use_resource, value=77               <-- resource still alive
// [Resource #3 "use_resource"] destroyed         <-- destroyed when use_resource() returns
//   (was the resource still alive after print_resource returned?)
//
// === end of main ===
// =============================================================================

// =============================================================================
// FIX GUIDE (read only after attempting fixes yourself)
//
// Bug A fix:
//   Replace:  Resource* r = new Resource("process_data", x);
//   With:     auto r = std::make_unique<Resource>("process_data", x);
//   Delete the manual `delete r;` line.
//   unique_ptr's destructor runs even when an exception unwinds the stack.
//
// Bug B fix:
//   In Publisher, change:
//       std::shared_ptr<Subscriber> subscriber_;
//   to:
//       std::weak_ptr<Subscriber> subscriber_;
//   Then to use it: if (auto s = subscriber_.lock()) { ... }
//
// Bug C fix:
//   Change the signature of print_resource to take by const reference:
//       void print_resource(const Resource& r)
//   or to take a raw (non-owning) pointer:
//       void print_resource(const Resource* r)
//   Remove the std::move at the call site.
//   Rule of thumb: if a function doesn't need to *own* the resource,
//   it should not take a smart pointer by value.
// =============================================================================
