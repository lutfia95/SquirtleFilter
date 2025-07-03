template <typename T>
bool SFilters::contains(const T& key) const {
    for (const auto& filter : filters) {
        if (filter.contains(key)) return true;
    }
    return false;
}

template <typename T>
std::vector<size_t> SFilters::matchFilters(const T& key) const {
    std::vector<size_t> matched;
    for (size_t i = 0; i < filters.size(); ++i) {
        if (filters[i].contains(key)) {
            matched.push_back(i);
        }
    }
    return matched;
}

