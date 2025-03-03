#ifndef STACK_H
#define STACK_H

#include <iostream>

template<typename T>
class Stack {
private:
    T* data;         // ������������ ������ ��� �������� ��������� �����
    int capacity;    // ������� ����������� �����
    int size;        // ������� ���������� ��������� � �����

    // ����������� ����������� ������� ��� �������������
    void resize() {
        capacity *= 2;                     // ����������� ������� � 2 ����
        T* newData = new T[capacity];      // ������ ����� ������ ������� �������
        for (int i = 0; i < size; ++i) {   // �������� ������ ��������
            newData[i] = data[i];
        }
        delete[] data;                     // ������� ������ ������
        data = newData;                    // ������������� ���������
    }

public:
    // �����������
    Stack() : data(nullptr), capacity(2), size(0) {
        data = new T[capacity]; // ���������� �������� ��������� ������
    }

    // ���������� (����������� ������)
    ~Stack() {
        delete[] data;
    }

    // �������� push: ��������� ������� � ����
    void push(const T& value) {
        if (size == capacity) {
            resize(); // ���� ����� ������������, ����������� ������
        }
        data[size++] = value; // ��������� ����� ������� � ����������� ������
    }

    // �������� pop: ������� ������� ������� �����
    void pop() {
        if (size == 0) {
            std::cerr << "Error: Attempt to pop from an empty stack\n";
            return;
        }
        --size; // ������ ��������� ������ �����
    }

    // �������� top: ���������� ������� ������� ����� (��� ��������)
    T top() const {
        if (size == 0) {
            std::cerr << "Error: Attempt to access top of an empty stack\n";
            exit(EXIT_FAILURE); // ��������� ���������� � ���������� �� ������
        }
        return data[size - 1]; // ���������� ������� �������
    }

    // �������� empty: ���������, ���� �� ����
    bool empty() const {
        return size == 0;
    }

    // ��������� ������� (�������������, ��� �����)
    int getSize() const {
        return size;
    }
};

#endif // STACK_H
