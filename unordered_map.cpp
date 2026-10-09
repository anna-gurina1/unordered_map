#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <utility>
#include <functional>
#include <cmath>
#include <cassert>

class unordered_map
{
    struct Node
    {
        int key;
        int value;
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

    Node **create_buckets(size_t count)
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
    unordered_map()
    {
        buckets_ = create_buckets(bucket_count_);
    };
    ~unordered_map()
    {
        clear();
        delete[] buckets_;
    };
    unordered_map(const std::pair<int, int> a[], size_t s)
    {
        if (s == 0)
        {
            buckets_ = create_buckets(bucket_count_);
            return;
        }
        buckets_ = create_buckets(bucket_count_);
        for (size_t i = 0; i < s; i++)
        {
            insert(a[i].first, a[i].second);
        }
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
                this->insert(node_other->key, node_other->value);
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
            if (node->key == k)
                return node->value;
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
                node->next = buckets_[hash(node->key)];
                buckets_[hash(node->key)] = node;
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
            if (node->key == key)
            {
                return false;
            }
            node = node->next;
        }
        if (float(size_ + 1) / bucket_count_ > max_load_factor_)
        {
            rehash(bucket_count_ * 2);
        }

        Node *node_new = new Node;
        node_new->key = key;
        node_new->value = value;
        node_new->next = buckets_[hash(key)];
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
            if (node->key == key)
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
                    int value = b.at(node->key);

                    if (node->value != value)
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
};

int main()
{

    unordered_map a;
    assert(a.empty());
    assert(a.size() == 0);

    assert(a.insert(1, 10));
    assert(a.insert(2, 20));
    assert(a.insert(3, 30));

    assert(a.size() == 3);
    assert(a.at(1) == 10);
    assert(a.at(2) == 20);
    assert(a.at(3) == 30);

    assert(!a.insert(2, 200));
    assert(a.size() == 3);
    assert(a.at(2) == 20);

    a.erase(2);
    assert(a.size() == 2);

    std::pair<int, int> data[] = {
        {5, 50},
        {6, 60},
        {7, 70}};

    unordered_map b(data, 3);

    assert(b.size() == 3);
    assert(b.at(5) == 50);
    assert(b.at(6) == 60);
    assert(b.at(7) == 70);

    unordered_map c(b);

    assert(unordered_map::is_equal(b, c));

    c.erase(5);

    assert(c.size() == 2);
    assert(b.size() == 3);
    assert(b.at(5) == 50);

    c.clear();
    assert(c.empty());
    assert(c.size() == 0);

    unordered_map d;
    d.insert(100, 1000);
    d.insert(200, 2000);

    unordered_map e;
    e.insert(300, 3000);

    d.swap(e);

    assert(d.size() == 1);
    assert(d.at(300) == 3000);

    assert(e.size() == 2);
    assert(e.at(100) == 1000);
    assert(e.at(200) == 2000);

    unordered_map f(2);
    f.insert(1, 100);
    f.insert(2, 200);
    f.insert(3, 300);
    f.insert(4, 400);

    f.rehash(20);

    assert(f.size() == 4);
    assert(f.at(1) == 100);
    assert(f.at(2) == 200);
    assert(f.at(3) == 300);
    assert(f.at(4) == 400);

    assert(f.load_factor() <= f.max_load_factor());

    unordered_map h;
    h.insert(10, 100);
    h.insert(20, 200);

    unordered_map i;
    i.insert(20, 200);
    i.insert(10, 100);

    assert(unordered_map::is_equal(h, i));

    i.erase(10);
    assert(!unordered_map::is_equal(h, i));

    unordered_map j;
    j.insert(11, 110);
    j.insert(22, 220);

    unordered_map k(std::move(j));

    assert(k.size() == 2);
    assert(k.at(11) == 110);
    assert(k.at(22) == 220);

    assert(j.empty());
    assert(j.size() == 0);

    assert(j.empty());

    j.insert(33, 330);
    assert(j.at(33) == 330);
    assert(j.size() == 1);

    return 0;
}
