// C++
#pragma once
#include <set>
#include <string>
#include <mutex>

class StringPool {
public:
    // Get the singleton instance
    static StringPool& instance() {
        static StringPool inst;
        return inst;
    }

    // Insert `s` if not present. Returns pointer to the stored string.
    const std::string* intern(const std::string& s) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto [it, inserted] = pool_.insert(s);
        return &*it;
    }

    // Overload to take an rvalue
    const std::string* intern(std::string&& s) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto [it, inserted] = pool_.insert(std::move(s));
        return &*it;
    }

    bool contains(const std::string& s) const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.find(s) != pool_.end();
    }

    std::size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }

private:
    StringPool() = default;
    ~StringPool() = default;
    StringPool(const StringPool&) = delete;
    StringPool& operator=(const StringPool&) = delete;

    mutable std::mutex mutex_;
    std::set<std::string> pool_;
};
