#ifndef MY_LIST_H
#define MY_LIST_H

#include <stdexcept> // ��� ��������� ���������� � ������� back()

template <typename T>
class MyList {
public:
    struct Node {
        T data;
        Node* next;
    };
    MyList() : head(nullptr), tail(nullptr), size_(0) {}
    void push_back(const T& value) {
        Node* new_node = new Node{ value, nullptr };
        if (tail) {tail->next = new_node;}
        else {head = new_node;}
        tail = new_node;
        ++size_;
    }
    bool empty() const {
        return size_ == 0;
    }
    class Iterator {
    public:
        Iterator(Node* node) : current(node) {}
        T& operator*() { return current->data; }
        Iterator& operator++() {
            if (current) {current = current->next;}
            return *this;
        }
        bool operator!=(const Iterator& other) const {return current != other.current;}
    private:
        Node* current;
    };
    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }
    size_t size() const { return size_; }

    void clear() {
        while (head) {
            Node* temp = head;
            head = head->next;
            delete temp;
        }
        tail = nullptr;
        size_ = 0;
    }

    T& operator[](size_t index) {
        if (index >= size_) {throw std::out_of_range("������ ������� �� ���������� ������� ������");}
        Node* current = head;
        for (size_t i = 0; i < index; ++i) {
            current = current->next;
        }
        return current->data;
    }
    const T& operator[](size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("������ ������� �� ���������� ������� ������");
        }
        Node* current = head;
        for (size_t i = 0; i < index; ++i) {
            current = current->next;
        }
        return current->data;
    }
    T& back() {
        if (empty()) {throw std::out_of_range("���������� �������� ��������� �������: ������ ����.");}
        return tail->data; // ���������� ������ �� ���� tail
    }
    const T& back() const {
        if (empty()) { throw std::out_of_range("���������� �������� ��������� �������: ������ ����.");}
        return tail->data; // ���������� ������ �� ���� tail (������ ��� ������)
    }
    Node* getHead() const { return head; }
private:
    Node* head;    // ��������� �� ������ ���� ������
    Node* tail;    // ��������� �� ��������� ���� ������
    size_t size_;  // ������ ������
};

#endif // MY_LIST_H
