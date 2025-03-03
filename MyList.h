#ifndef MY_LIST_H
#define MY_LIST_H

#include <stdexcept> // для обработки исключений в функции back()

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
        if (index >= size_) {throw std::out_of_range("Индекс выходит за допустимые границы списка");}
        Node* current = head;
        for (size_t i = 0; i < index; ++i) {
            current = current->next;
        }
        return current->data;
    }
    const T& operator[](size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Индекс выходит за допустимые границы списка");
        }
        Node* current = head;
        for (size_t i = 0; i < index; ++i) {
            current = current->next;
        }
        return current->data;
    }
    T& back() {
        if (empty()) {throw std::out_of_range("Невозможно получить последний элемент: список пуст.");}
        return tail->data; // Возвращаем данные из узла tail
    }
    const T& back() const {
        if (empty()) { throw std::out_of_range("Невозможно получить последний элемент: список пуст.");}
        return tail->data; // Возвращаем данные из узла tail (только для чтения)
    }
    Node* getHead() const { return head; }
private:
    Node* head;    // Указатель на первый узел списка
    Node* tail;    // Указатель на последний узел списка
    size_t size_;  // Размер списка
};

#endif // MY_LIST_H
