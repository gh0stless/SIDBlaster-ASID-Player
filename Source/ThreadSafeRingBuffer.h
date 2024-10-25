#pragma once
#include <atomic>
#include <JuceHeader.h>

template <typename T>
class ThreadSafeRingBuffer {
public:
    ThreadSafeRingBuffer(size_t size)
        : maxSize(size), head(0), tail(0), isFull(false) {
        buffer = new T[maxSize];  // Fester Speicher für den Ringpuffer
    }

    ~ThreadSafeRingBuffer() {
        delete[] buffer;  // Speicher freigeben
    }

    // Add a new value to the ring buffer
    void add(const T& value) {
        buffer[head.load()] = value;  // Schreibe den Wert in das Puffer-Array
        auto nextHead = (head.load() + 1) % maxSize;

        if (nextHead == tail.load()) {  // Puffer ist voll, schiebe den Tail-Zeiger weiter
            tail.store((tail.load() + 1) % maxSize);
            isFull.store(true);
        }

        head.store(nextHead);
    }

    // Remove an item from the ring buffer
    bool remove(T& value) {
        if (isEmpty()) {
            return false;
        }

        value = buffer[tail.load()];
        tail.store((tail.load() + 1) % maxSize);
        isFull.store(false);

        return true;
    }

    // Check if the buffer is empty
    bool isEmpty() const {
        return (!isFull.load() && (head.load() == tail.load()));
    }

    // Get the current size of the buffer
    int size() const {
        if (isFull.load()) {
            return maxSize;
        }

        if (head.load() >= tail.load()) {
            return head.load() - tail.load();
        }
        else {
            return maxSize + head.load() - tail.load();
        }
    }

private:
    T* buffer;  // C-Array für den Puffer
    const size_t maxSize;  // Maximale Größe des Puffers
    std::atomic<size_t> head;  // Kopfzeiger, atomic für Thread-Sicherheit
    std::atomic<size_t> tail;  // Schwanzzeiger, atomic für Thread-Sicherheit
    std::atomic<bool> isFull;  // Status, ob der Puffer voll ist, atomic für Thread-Sicherheit
};
