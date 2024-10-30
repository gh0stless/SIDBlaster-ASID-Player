/*
  ==============================================================================

    ThreadSafeRingBuffer.h
    Created: 22 Oct 2024 10:28:42am
    Author:  Andreas Schumm (gh0stless)

  ==============================================================================
*/
#pragma once
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
        juce::ScopedLock lock(mutex);  // Sperrt den kritischen Abschnitt
        buffer[head] = value;
        auto nextHead = (head + 1) % maxSize;

        if (nextHead == tail) {  // Puffer ist voll, schiebe den Tail-Zeiger weiter
            tail = (tail + 1) % maxSize;
            isFull = true;
        }

        head = nextHead;
    }

    // Remove an item from the ring buffer
    bool remove(T& value) {
        juce::ScopedLock lock(mutex);  // Sperrt den kritischen Abschnitt
        if (isEmpty()) {
            return false;
        }

        value = buffer[tail];
        tail = (tail + 1) % maxSize;
        isFull = false;

        return true;
    }

    // Check if the buffer is empty
    bool isEmpty() const {
        juce::ScopedLock lock(mutex);  // Sperrt den kritischen Abschnitt
        return (!isFull && (head == tail));
    }

private:
    T* buffer;  // C-Array für den Puffer
    const size_t maxSize;  // Maximale Größe des Puffers
    size_t head;  // Kopfzeiger
    size_t tail;  // Schwanzzeiger
    bool isFull;  // Status, ob der Puffer voll ist
    mutable juce::CriticalSection mutex;  // JUCE Mutex für Thread-Sicherheit
};
