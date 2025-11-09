#pragma once

#include <string>
#include <vector>

struct SessionItem {
    std::string label;
    // placeholders for n, e, c, p, q as strings for now
    std::string n, e, c, p, q;
    std::string notes;
};

class SessionState {
public:
    size_t size() const { return items_.size(); }
    void add(const SessionItem &it) { items_.push_back(it); }
private:
    std::vector<SessionItem> items_;
};

