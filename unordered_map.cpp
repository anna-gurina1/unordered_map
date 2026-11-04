#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <functional>
#include <cmath>
#include <cassert>
#include <initializer_list>
#include <iterator>

class unordered_map
{
    struct Node
    {
        std::pair<const int, int> element;
        Node *next;
    };

    Node **buckets_{nullptr};
    size_t bucket_count_{8};
    size_t size_{0};
    float max_load_factor_{1};

    void clear_list(Node *a)
    {
        if (a == nullptr)
        {
            return;
        }
        clear_list(a->next);
        delete a;
    }

    Node **create_buckets(const size_t count)
    {
        Node **new_backets = new Node *[count];
        for (size_t i = 0; i < count; i++)
        {
            new_backets[i] = nullptr;
        }
        return new_backets;
    };

    size_t hash(int key) const
    {
        std::hash<int> num;
        return num(key) % bucket_count_;
    }

public:
    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<const int, int>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type *;
        using reference = value_type &;

        ~iterator() = default;
        iterator()
        {
            current_ = nullptr;
            buckets_ = nullptr;
            bucket_index_ = 0;
            bucket_count_ = 0;
        }
        iterator(const iterator &other)
        {
            current_ = other.current_;
            buckets_ = other.buckets_;
            bucket_index_ = other.bucket_index_;
            bucket_count_ = other.bucket_count_;
        }

        iterator &operator++()
        {
            if (current_ == nullptr)
            {
                return *this;
            }
            if (current_->next != nullptr)
            {
                current_ = current_->next;
                return *this;
            }
            bucket_index_++;
            while (bucket_index_ < bucket_count_)
            {
                if (buckets_[bucket_index_] != nullptr)
                {
                    current_ = buckets_[bucket_index_];
                    return *this;
                }

                ++bucket_index_;
            }
            current_ = nullptr;
            return *this;
        }
        iterator operator++(int)
        {
            iterator old = *this;
            ++(*this);
            return old;
        }

        iterator(Node *current, Node **buckets, size_t bucket_index, size_t bucket_count)
        {
            current_ = current;
            buckets_ = buckets;
            bucket_index_ = bucket_index;
            bucket_count_ = bucket_count;
        }
        std::pair<const int, int> &operator*()
        {
            return current_->element;
        }
        std::pair<const int, int> *operator->()
        {
            return &current_->element;
        }
        friend bool operator==(const iterator &a, const iterator &b)
        {
            return a.current_ == b.current_ && a.buckets_ == b.buckets_;
        }
        friend bool operator!=(const iterator &a, const iterator &b)
        {
            return !(a == b);
        }

    private:
        Node *current_;
        Node **buckets_;
        size_t bucket_index_;
        size_t bucket_count_;
    };

    unordered_map()
    {
        buckets_ = create_buckets(bucket_count_);
    };
    ~unordered_map()
    {
        clear();
        delete[] buckets_;
    };
    unordered_map(size_t a)
    {
        if (a == 0)
        {
            a = 8;
        }

        bucket_count_ = a;
        buckets_ = create_buckets(a);
    };
    unordered_map(const unordered_map &a)
    {
        bucket_count_ = a.bucket_count_;
        buckets_ = create_buckets(bucket_count_);
        max_load_factor_ = a.max_load_factor_;
        for (size_t i = 0; i < bucket_count_; i++)
        {
            Node *node_other = a.buckets_[i];
            while (node_other != nullptr)
            {
                this->insert(node_other->element.first, node_other->element.second);
                node_other = node_other->next;
            }
        }
    };
    unordered_map(unordered_map &&a)
    {
        buckets_ = std::exchange(a.buckets_, nullptr);
        bucket_count_ = std::exchange(a.bucket_count_, 0);
        size_ = std::exchange(a.size_, 0);
        max_load_factor_ = a.max_load_factor_;
    };

    float load_factor() const
    {
        if (bucket_count_ == 0)
            return 0;
        return float(size_) / float(bucket_count_);
    }
    float max_load_factor() const
    {
        return max_load_factor_;
    };
    void max_load_factor(float value)
    {
        if (value <= 0)
        {
            return;
        }
        max_load_factor_ = value;
    };

    void clear()
    {
        for (size_t i = 0; i < bucket_count_; i++)
        {
            clear_list(buckets_[i]);
            buckets_[i] = nullptr;
        }
        size_ = 0;
    };

    int at(int k) const
    {
        if (bucket_count_ == 0)
            throw std::out_of_range("Key not found");
        Node *node = buckets_[hash(k)];
        while (node != nullptr)
        {
            if (node->element.first == k)
                return node->element.second;
            node = node->next;
        };
        throw std::out_of_range("Key not found");
    };

    bool empty() const
    {
        return !size_;
    };
    void swap(unordered_map &a)
    {
        size_t tmp = size_;
        size_ = a.size_;
        a.size_ = tmp;

        Node **tmp2 = buckets_;
        buckets_ = a.buckets_;
        a.buckets_ = tmp2;

        size_t tmp3 = bucket_count_;
        bucket_count_ = a.bucket_count_;
        a.bucket_count_ = tmp3;

        float tmp4 = max_load_factor_;
        max_load_factor_ = a.max_load_factor_;
        a.max_load_factor_ = tmp4;
    };

    void rehash(size_t a)
    {
        Node **buckets_old = buckets_;
        size_t bucket_count_old = bucket_count_;

        size_t min = std::ceil(float(size_) / max_load_factor_);

        bucket_count_ = std::max({size_t(1), min, a});
        buckets_ = create_buckets(bucket_count_);
        for (size_t i = 0; i < bucket_count_old; i++)
        {

            Node *node = buckets_old[i];
            while (node != nullptr)
            {
                Node *next = node->next;
                node->next = buckets_[hash(node->element.first)];
                buckets_[hash(node->element.first)] = node;
                node = next;
            }
        }
        delete[] buckets_old;
    }

    bool insert(int key, int value)
    {
        if (bucket_count_ == 0)
        {
            bucket_count_ = 8;
            buckets_ = create_buckets(bucket_count_);
        }
        Node *node = buckets_[hash(key)];

        while (node != nullptr)
        {
            if (node->element.first == key)
            {
                return false;
            }
            node = node->next;
        }
        if (float(size_ + 1) / bucket_count_ > max_load_factor_)
        {
            rehash(bucket_count_ * 2);
        }

        Node *node_new = new Node{{key, value}, buckets_[hash(key)]};
        buckets_[hash(key)] = node_new;

        size_++;
        return true;
    };

    void erase(int key)
    {
        if (bucket_count_ == 0)
            return;
        Node *node = buckets_[hash(key)];
        Node *prev = nullptr;
        while (node != nullptr)
        {
            if (node->element.first == key)
            {
                if (prev == nullptr)
                {
                    buckets_[hash(key)] = node->next;
                    delete node;
                    size_--;
                    return;
                }

                prev->next = node->next;
                delete node;
                size_--;
                return;
            }
            prev = node;
            node = node->next;
        }
    }

    size_t size() const
    {
        return size_;
    }

    static bool is_equal(const unordered_map &a, const unordered_map &b)
    {
        if (a.size() != b.size())
            return false;

        for (size_t i = 0; i < a.bucket_count_; i++)
        {
            Node *node = a.buckets_[i];

            while (node != nullptr)
            {
                try
                {
                    int value = b.at(node->element.first);

                    if (node->element.second != value)
                    {
                        return false;
                    }
                }
                catch (const std::out_of_range &e)
                {
                    return false;
                }
                node = node->next;
            }
        }
        return true;
    };

    unordered_map(std::initializer_list<std::pair<const int, int>> a)
    {
        buckets_ = create_buckets(bucket_count_);
        for (const auto &element : a)
        {
            insert(element.first, element.second);
        }
    }

    unordered_map(const iterator &begin, const iterator &end)
    {
        buckets_ = create_buckets(bucket_count_);

        for (iterator i = begin; i != end; i++)
        {
            this->insert(i->first, i->second);
        }
    }
    auto end()
    {
        iterator it(nullptr, buckets_, bucket_count_, bucket_count_);
        return it;
    }

    auto begin()
    {
        size_t i = 0;
        while (i < bucket_count_ && this->buckets_[i] == nullptr)
        {
            i++;
        }

        if (i < bucket_count_)
        {
            return iterator(buckets_[i], buckets_, i, bucket_count_);
        }
        return end();
    }

    void assign(const iterator &begin, const iterator &end)
    {

        this->clear();
        for (iterator it = begin; it != end; it++)
        {
            this->insert(it->first, it->second);
        }
    }
};

int main(int argc, char const *argv[])
{
    unordered_map a{{19, 190}, {47, 470}, {74, 740}, {91, 910}};

    // Iterate through all elements
    for (auto it = a.begin(); it != a.end(); ++it)
        std::cout << it->first << ":" << it->second << " ";

    std::cout << '\n';

    // Range constructor
    unordered_map b(a.begin(), a.end());
    assert(unordered_map::is_equal(a, b));

    // Range-based for
    for (auto &&it : b)
        std::cout << it.first << ":" << it.second << " ";

    std::cout << '\n';

    // Assign from iterator range
    unordered_map c;
    c.assign(b.begin(), b.end());
    assert(unordered_map::is_equal(c, b));

    std::cout << '\n';

    // test standart algorithms
    [](const auto &element)
    {
        return element.second > 0;
    };

    bool all_positive = std::all_of(
        a.begin(),
        a.end(),
        [](const auto &element)
        {
            return element.second > 0;
        });

    std::cout << all_positive << "\n";

    bool any_greater_500 = std::any_of(
        a.begin(),
        a.end(),
        [](const auto &element)
        {
            return element.second > 500;
        });

    std::cout << "Any value is greater than 500: "
              << any_greater_500 << '\n';

    auto found = std::find_if(
        a.begin(),
        a.end(),
        [](const auto &element)
        {
            return element.first == 74;
        });

    if (found != a.end())
    {
        std::cout << "Found: "
                  << found->first << ":" << found->second << '\n';
    }
    else
    {
        std::cout << "Element not found\n";
    }

    // Count values greater than 400
    auto count = std::count_if(
        a.begin(),
        a.end(),
        [](const auto &element)
        {
            return element.second > 400;
        });

    std::cout << "Values greater than 400: "
              << count << '\n';

    return 0;
}
