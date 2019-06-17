#include <cstddef>
#include <iostream>

const size_t ROTATE_FACTOR = 2;

template<class T>
class Set {
private:
    struct Node;
    Node* root;
    size_t cnt;

    int height(Node* cur) const {
        if (!cur)
            return 0;
        return cur->height;
    }

    int factor(Node* cur) const {
        return height(cur->right) - height(cur->left);
    }

    void fix(Node* cur) {
        cur->height = std::max(height(cur->left), height(cur->right)) + 1;
    }

    void hang_left(Node* cur, Node* other) {
        cur->left = other;
        if (other)
            other->prev = cur;
    }

    void hang_right(Node* cur, Node* other) {
        cur->right = other;
        if (other)
            other->prev = cur;
    }

    Node* rotate_right(Node* cur) {
        Node* left_child = cur->left;
        hang_left(cur, left_child->right);
        hang_right(left_child, cur);
        fix(cur);
        fix(left_child);
        return left_child;
    }

    Node* rotate_left(Node* cur) {
        Node* right_child = cur->right;
        hang_right(cur, right_child->left);
        hang_left(right_child, cur);
        fix(cur);
        fix(right_child);
        return right_child;
    }

    Node* balance(Node* cur) {
        fix(cur);
        if (factor(cur) == ROTATE_FACTOR) {
            if (factor(cur->right) < 0)
                hang_right(cur, rotate_right(cur->right));
            return rotate_left(cur);
        }
        if (factor(cur) == -ROTATE_FACTOR) {
            if (factor(cur->left) > 0)
                hang_left(cur, rotate_left(cur->left));
            return rotate_right(cur);
        }
        return cur;
    }

    Node* insert(Node* cur, const T& elem) {
        if (!cur) {
            ++cnt;
            return new Node(elem);
        }
        if (elem < cur->key)
            hang_left(cur, insert(cur->left, elem));
        else if (cur->key < elem)
            hang_right(cur, insert(cur->right, elem));
        return balance(cur);
    }

    Node* find_min(Node* cur) const {
        if (cur->left)
            return find_min(cur->left);
        return cur;
    }

    Node* find_max(Node* cur) const {
        if (!cur)
            return nullptr;
        if (cur->right)
            return find_max(cur->right);
        return cur;
    }

    Node* erase_min(Node* cur) {
        if (!cur->left)
            return cur->right;
        hang_left(cur, erase_min(cur->left));
        return balance(cur);
    }

    Node* erase(Node* cur, const T& elem) {
        if (!cur)
            return nullptr;
        if (elem < cur->key)
            hang_left(cur, erase(cur->left, elem));
        else if (cur->key < elem)
            hang_right(cur, erase(cur->right, elem));
        else {
            --cnt;
            Node* left_child = cur->left;
            Node* right_child = cur->right;
            cur->left = cur->right = nullptr;
            delete cur;
            if (!right_child)
                return left_child;
            Node* min_elem = find_min(right_child);
            hang_right(min_elem, erase_min(right_child));
            hang_left(min_elem, left_child);
            return balance(min_elem);
        }
        return balance(cur);
    }

    Node* find(Node* cur, const T& elem) const {
        if (!cur)
            return nullptr;
        if (elem < cur->key)
            return find(cur->left, elem);
        else if (cur->key < elem)
            return find(cur->right, elem);
        return cur;
    }

    Node* lower_bound(Node* cur, const T& elem) const {
        if (!cur)
            return nullptr;
        if (cur->key < elem)
            return lower_bound(cur->right, elem);
        Node* left_bound = lower_bound(cur->left, elem);
        if (left_bound)
            return left_bound;
        return cur;
    }

public:
    class iterator;

    Set() : root(nullptr), cnt(0) {}

    ~Set() {
        delete root;
    }

    void clear() {
        delete root;
        root = nullptr;
        cnt = 0;
    }

    size_t size() const {
        return cnt;
    }

    bool empty() const {
        return cnt == 0;
    }

    void insert(const T& elem) {
        root = insert(root, elem);
        root->prev = nullptr;
    }

    void erase(const T& elem) {
        root = erase(root, elem);
        if (root)
            root->prev = nullptr;
    }

    template<typename Iterator>
    Set(Iterator first, Iterator last) : Set() {
        while (first != last)
            insert(*first++);
    }

    Set(std::initializer_list<T> elems) : Set() {
        for (const T& elem : elems)
            insert(elem);
    }

    Set(const Set& other) : Set() {
        for (const T& elem : other)
            insert(elem);
    }

    Set& operator=(const Set& other) {
        if (this != &other) {
            clear();
            for (const T& elem : other)
                insert(elem);
        }
        return *this;
    }

    iterator find(const T& elem) const {
        return iterator(find_max(root), find(root, elem));
    }

    iterator lower_bound(const T& elem) const {
        return iterator(find_max(root), lower_bound(root, elem));
    }

    iterator begin() const {
        if (!root)
            return iterator(find_max(root));
        return iterator(find_max(root), find_min(root));
    }

    iterator end() const {
        return iterator(find_max(root));
    }
};

template<class T>
struct Set<T>::Node {
    T key;
    unsigned char height;
    Node* left;
    Node* right;
    Node* prev;

    Node(T elem) : key(elem), height(1), left(nullptr), right(nullptr), prev(nullptr) {}

    ~Node() {
        delete left;
        delete right;
    }
};

template<class T>
class Set<T>::iterator {
private:
    Node* last;
    Node* cur;
public:
    iterator(Node* last_node = nullptr, Node* cur_node = nullptr) : last(last_node), cur(cur_node) {}

    iterator(const iterator& other) : last(other.last), cur(other.cur) {}

    iterator operator++() {
        if (cur->right) {
            cur = cur->right;
            while (cur->left)
                cur = cur->left;
        } else {
            while (cur) {
                if (cur->prev && cur->prev->left == cur) {
                    cur = cur->prev;
                    break;
                }
                cur = cur->prev;
            }
        }
        return *this;
    }

    iterator operator++(int) {
        iterator prev = *this;
        operator++();
        return prev;
    }

    iterator operator--() {
        if (!cur)
            cur = last;
        else if (cur->left) {
            cur = cur->left;
            while (cur->right)
                cur = cur->right;
        } else {
            while (cur) {
                if (cur->prev && cur->prev->right == cur) {
                    cur = cur->prev;
                    break;
                }
                cur = cur->prev;
            }
        }
        return *this;
    }

    iterator operator--(int) {
        iterator prev = *this;
        operator--();
        return prev;
    }

    T operator*() const {
        return cur->key;
    }

    const T* operator->() const {
        return &cur->key;
    }

    bool operator==(const iterator& other) {
        return cur == other.cur;
    }

    bool operator!=(const iterator& other) {
        return !(*this == other);
    }
};
