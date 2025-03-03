#ifndef STACK_H
#define STACK_H

#include <iostream>

template<typename T>
class Stack {
private:
    T* data;         // Динамический массив для хранения элементов стека
    int capacity;    // Текущая вместимость стека
    int size;        // Текущее количество элементов в стеке

    // Увеличивает вместимость массива при необходимости
    void resize() {
        capacity *= 2;                     // Увеличиваем ёмкость в 2 раза
        T* newData = new T[capacity];      // Создаём новый массив большей ёмкости
        for (int i = 0; i < size; ++i) {   // Копируем старые элементы
            newData[i] = data[i];
        }
        delete[] data;                     // Удаляем старый массив
        data = newData;                    // Переназначаем указатель
    }

public:
    // Конструктор
    Stack() : data(nullptr), capacity(2), size(0) {
        data = new T[capacity]; // Изначально выделяем небольшой массив
    }

    // Деструктор (освобождаем память)
    ~Stack() {
        delete[] data;
    }

    // Операция push: добавляет элемент в стек
    void push(const T& value) {
        if (size == capacity) {
            resize(); // Если места недостаточно, увеличиваем массив
        }
        data[size++] = value; // Добавляем новый элемент и увеличиваем размер
    }

    // Операция pop: удаляет верхний элемент стека
    void pop() {
        if (size == 0) {
            std::cerr << "Error: Attempt to pop from an empty stack\n";
            return;
        }
        --size; // Просто уменьшаем размер стека
    }

    // Операция top: возвращает верхний элемент стека (без удаления)
    T top() const {
        if (size == 0) {
            std::cerr << "Error: Attempt to access top of an empty stack\n";
            exit(EXIT_FAILURE); // Завершаем выполнение с сообщением об ошибке
        }
        return data[size - 1]; // Возвращаем верхний элемент
    }

    // Операция empty: проверяет, пуст ли стек
    bool empty() const {
        return size == 0;
    }

    // Получение размера (необязательно, как бонус)
    int getSize() const {
        return size;
    }
};

#endif // STACK_H
