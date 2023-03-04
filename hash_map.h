#include <vector>
#include <memory>
#include <stdexcept>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap{
    private:

    const int NORMAL_RESERVE_FROM_SIZE = 4;
    const int MIN_RESERVE_FROM_SIZE = 2;
    const size_t MIN_RESERVE = 4;

    std::size_t size_;
    std::size_t reserve_;
    std::vector<std::vector<std::shared_ptr<std::pair<const KeyType, ValueType>>>> map_;
    Hash hasher_;

    std::vector<size_t> give_some_position(size_t hash) const {
        std::vector<size_t> it(NORMAL_RESERVE_FROM_SIZE);
        for (int i = 0; i < NORMAL_RESERVE_FROM_SIZE; ++i) {
            it[i] = ((hash * i + 46241 * i) * 33) % reserve_;
        }
        return it;
    }

    size_t choos_position(const std::vector<size_t> &hashs) const {
        size_t min_size =  map_[hashs[0]].size();
        for (size_t i = 1; i < hashs.size(); ++i) {
            min_size = std::min(min_size, map_[hashs[i]].size());
        }
        for (size_t i = 0; i < hashs.size(); ++i) {
            if (min_size == map_[hashs[i]].size()) {
                return hashs[i];
            }
        }
        return 0;
    }

    void rebuild() {
        reserve_ = std::max(MIN_RESERVE, size_ * NORMAL_RESERVE_FROM_SIZE);
        std::vector<std::vector<std::shared_ptr<std::pair<const KeyType, ValueType>>>> old_map(reserve_);
        swap(old_map, map_);
        for (size_t i = 0; i < old_map.size(); ++i) {
            for (size_t q = 0; q < old_map[i].size(); ++q) {
                size_t position = choos_position(give_some_position(hasher_(old_map[i][q]->first)));
                map_[position].push_back(move(old_map[i][q]));
            }
        }
    }

    std::pair<size_t, size_t> find_position(const KeyType &key, const std::vector<size_t> &hashs) const {
        for (size_t i = 0; i < hashs.size(); ++i) {
            size_t position = hashs[i];
            for (size_t j = 0; j < map_[position].size(); ++j) {
                if (map_[position][j]->first == key) {
                    return {position, j};
                }
            }
        }
        return {reserve_, 0};
    }

    public:

    HashMap() {
        size_ = 0;
        reserve_ = MIN_RESERVE;
        map_ = std::vector<std::vector<std::shared_ptr<std::pair<const KeyType, ValueType>>>> (reserve_);
    }

    HashMap(Hash hasher) : hasher_(hasher) {
        size_ = 0;
        reserve_ = MIN_RESERVE;
        map_ = std::vector<std::vector<std::shared_ptr<std::pair<const KeyType, ValueType>>>> (reserve_);
    }

    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>> &list) {
        size_ = list.size();
        reserve_ = MIN_RESERVE;
        map_ = std::vector<std::vector<std::shared_ptr<std::pair<const KeyType, ValueType>>>> (reserve_);
        for (auto it = list.begin(); it != list.end(); ++it) {
            map_[0].push_back(std::shared_ptr<std::pair<const KeyType, ValueType>>(
                new std::pair<const KeyType, ValueType>{it->first, it->second}));
        }
        rebuild();
    }

    template<class T>
    HashMap(T it, T end){
        size_ = 0;
        reserve_ = MIN_RESERVE;
        map_ = std::vector<std::vector<std::shared_ptr<std::pair<const KeyType, ValueType>>>> (reserve_);
        for (; it != end; ++it) {
            ++size_;
            map_[0].push_back(std::shared_ptr<std::pair<const KeyType, ValueType>>(
                new std::pair<const KeyType, ValueType>{it->first, it->second}));
        }
        rebuild();
    }

    template<class T>
    HashMap(T it, T end, Hash hasher) : hasher_(hasher) {
        size_ = 0;
        reserve_ = MIN_RESERVE;
        map_ = std::vector<std::vector<std::shared_ptr<std::pair<const KeyType, ValueType>>>> (reserve_);
        for (; it != end; ++it) {
            ++size_;
            map_[0].push_back(std::shared_ptr<std::pair<const KeyType, ValueType>>(
                new std::pair<const KeyType, ValueType>{it->first, it->second}));
        }
        rebuild();
    }

    HashMap(const std::initializer_list<std::pair<KeyType, ValueType>> &list, Hash hasher) : hasher_(hasher) {
        size_ = list.size();
        reserve_ = MIN_RESERVE;
        map_ = std::vector<std::vector<std::shared_ptr<std::pair<const KeyType, ValueType>>>> (reserve_);
        for (auto it = list.begin(); it != list.end(); ++it) {
            map_[0].push_back(std::shared_ptr<std::pair<const KeyType, ValueType>>(
                new std::pair<const KeyType, ValueType>{it->first, it->second}));
        }
        rebuild();
    }

    ValueType &operator[](const KeyType &key) {
        std::vector<size_t> hashs = give_some_position(hasher_(key));
        std::pair<size_t, size_t> position = find_position(key, hashs);
        if (position.first != reserve_) {
            return map_[position.first][position.second]->second;
        }
        ++size_;
        if (size_ * MIN_RESERVE_FROM_SIZE > reserve_) {
            rebuild();
            hashs = give_some_position(hasher_(key));
        }
        size_t backet = choos_position(hashs);
        map_[backet].push_back(std::shared_ptr<std::pair<const KeyType, ValueType>>(
            new std::pair<const KeyType, ValueType>{key, ValueType()}));
        return map_[backet].back()->second;
    }

    HashMap<KeyType, ValueType> &operator=(const HashMap<KeyType, ValueType> &b) {
        size_ = b.size_;
        reserve_ = b.reserve_;
        map_ = b.map_;
        hasher_ = b.hasher_;
        return *this;
    };

    size_t size() const {
        return size_;
    }

    bool empty() const {
        if (size_ == 0) {
            return true;
        }
        return false;
    }

    void erase(const KeyType &key) {
        std::vector<size_t> hashs = give_some_position(hasher_(key));
        std::pair<size_t, size_t> position = find_position(key, hashs);
        if (position.first != reserve_) {
            map_[position.first][position.second] = move(map_[position.first].back());
            map_[position.first].pop_back();
            --size_;
        }
    }

    class const_iterator {
    private:

        size_t backet_nomber_;
        size_t nomber_in_backet_;
        const HashMap* parent_;

    public:

        const_iterator() = default;

        const_iterator(size_t backet_nomber, size_t nomber_in_backet, const HashMap* parent) : parent_(parent) {
            backet_nomber_ = backet_nomber;
            nomber_in_backet_ = nomber_in_backet;
        }

        const_iterator(const HashMap* parent) : parent_(parent) {
            backet_nomber_ = parent->reserve_;
            nomber_in_backet_ = 0;
        }

        bool operator==(const const_iterator& b) const {
            if (backet_nomber_ == b.backet_nomber_ && nomber_in_backet_ == b.nomber_in_backet_) {
                return true;
            }
            else {
                return false;
            }
        }

        bool operator!=(const const_iterator& b) const {
            if (backet_nomber_ == b.backet_nomber_ && nomber_in_backet_ == b.nomber_in_backet_) {
                return false;
            }
            else {
                return true;
            }
        }

        const std::pair<const KeyType, ValueType> &operator*() const {
            return *parent_->map_[backet_nomber_][nomber_in_backet_];
        }

        const std::pair<const KeyType, ValueType> *operator->() const {
            return &(*parent_->map_[backet_nomber_][nomber_in_backet_]);
        }

        const_iterator operator++(int) {
            const_iterator it = *this;
            if (parent_->map_[backet_nomber_].size() > nomber_in_backet_ + 1) {
                ++nomber_in_backet_;
                return it;
            }
            ++backet_nomber_;
            nomber_in_backet_ = 0;
            for(; backet_nomber_ < parent_->reserve_; ++backet_nomber_) {
                if (!parent_->map_[backet_nomber_].empty()) {
                    return it;
                }
            }
            return it;
        }

        const_iterator &operator++() {
            if (parent_->map_[backet_nomber_].size() > nomber_in_backet_ + 1) {
                ++nomber_in_backet_;
                return *this;
            }
            ++backet_nomber_;
            nomber_in_backet_ = 0;
            for(; backet_nomber_ < parent_->reserve_; ++backet_nomber_) {
                if (!parent_->map_[backet_nomber_].empty()) {
                    return *this;
                }
            }
            return *this;
        }
    };

    class iterator {
    private:

        size_t backet_nomber_;
        size_t nomber_in_backet_;
        const HashMap* parent_;

    public:

        iterator() = default;

        iterator(size_t backet_nomber, size_t nomber_in_backet, const HashMap* parent) : parent_(parent) {
            backet_nomber_ = backet_nomber;
            nomber_in_backet_ = nomber_in_backet;
        }

        iterator(const HashMap* parent) : parent_(parent) {
            backet_nomber_ = parent->reserve_;
            nomber_in_backet_ = 0;
        }

        bool operator==(const iterator& b) const {
            if (backet_nomber_ == b.backet_nomber_ && nomber_in_backet_ == b.nomber_in_backet_) {
                return true;
            }
            else {
                return false;
            }
        }

        bool operator!=(const iterator& b) const {
            if (backet_nomber_ == b.backet_nomber_ && nomber_in_backet_ == b.nomber_in_backet_) {
                return false;
            }
            else {
                return true;
            }
        }

        std::pair<const KeyType, ValueType> &operator*() const {
            return *parent_->map_[backet_nomber_][nomber_in_backet_];
        }

        std::pair<const KeyType, ValueType> *operator->() const {
            return &(*parent_->map_[backet_nomber_][nomber_in_backet_]);
        }

        iterator operator++(int) {
            iterator it = *this;
            if (parent_->map_[backet_nomber_].size() > nomber_in_backet_ + 1) {
                ++nomber_in_backet_;
                return it;
            }
            ++backet_nomber_;
            nomber_in_backet_ = 0;
            for(; backet_nomber_ < parent_->reserve_; ++backet_nomber_) {
                if (!parent_->map_[backet_nomber_].empty()) {
                    return it;
                }
            }
            return it;
        }

        iterator &operator++() {
            if (parent_->map_[backet_nomber_].size() > nomber_in_backet_ + 1) {
                ++nomber_in_backet_;
                return *this;
            }
            ++backet_nomber_;
            nomber_in_backet_ = 0;
            for(; backet_nomber_ < parent_->reserve_; ++backet_nomber_) {
                if (!parent_->map_[backet_nomber_].empty()) {
                    return *this;
                }
            }
            return *this;
        }
    };

    iterator find(const KeyType &key) {
        std::pair<size_t, size_t> position = find_position(key, give_some_position(hasher_(key)));
        if (position.first == reserve_) {
            return iterator(this);
        }
        return iterator(position.first, position.second, this);
    }

    const_iterator find(const KeyType &key) const {
        std::pair<size_t, size_t> position = find_position(key, give_some_position(hasher_(key)));
        if (position.first == reserve_) {
            return const_iterator(this);
        }
        return const_iterator(position.first, position.second, this);
    }

    iterator begin() {
        if(size_ == 0) {
            return iterator(this);
        }
        for(size_t i = 0; i < reserve_; ++i) {
            if (!map_[i].empty()) {
                return iterator(i, 0, this);
            }
        }
        return iterator(this);
    }

    const_iterator begin() const {
        if(size_ == 0) {
            return const_iterator(this);
        }
        for(size_t i = 0; i < reserve_; ++i) {
            if (!map_[i].empty()) {
                return const_iterator(i, 0, this);
            }
        }
        return const_iterator(this);
    }

    iterator end() {
        return iterator(this);
    }

    const_iterator end() const {
        return const_iterator(this);
    }

    void clear() {
        size_ = 0;
        reserve_ = MIN_RESERVE;
        map_ = std::vector<std::vector<std::shared_ptr<std::pair<const KeyType, ValueType>>>> (reserve_);
    }

    Hash hash_function() const {
        return hasher_;
    }

    const ValueType &at(const KeyType &key) const {
        std::vector<size_t> hashs = give_some_position(hasher_(key));
        std::pair<size_t, size_t> position = find_position(key, hashs);
        if (position.first != reserve_) {
            return map_[position.first][position.second]->second;
        }
        throw std::out_of_range("err");
    }

    void insert(const std::pair<KeyType, ValueType> &pair) {
        std::vector<size_t> hashs = give_some_position(hasher_(pair.first));
        std::pair<size_t, size_t> position = find_position(pair.first, hashs);
        if (position.first != reserve_) {
            return;
        }
        ++size_;
        if (size_ * MIN_RESERVE_FROM_SIZE > reserve_) {
            rebuild();
            hashs = give_some_position(hasher_(pair.first));
        }
        size_t backet = choos_position(hashs);
        map_[backet].push_back(std::shared_ptr<std::pair<const KeyType, ValueType>>(
            new std::pair<const KeyType, ValueType>{pair.first, pair.second}));
    }
};