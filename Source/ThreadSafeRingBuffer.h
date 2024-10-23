/*
  ==============================================================================

    ThreadSafeRingBuffer.h
    Created: 22 Oct 2024 8:09:34am
    Author:  andre

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

    void add(T value) {
        juce::ScopedLock lock(mutex); // Thread-sicherer Zugriff

        buffer[head] = value;
        head = (head + 1) % maxSize;

        if (isFull) {
            // Überschreibt das älteste Element
            tail = (tail + 1) % maxSize;
        }

        isFull = (head == tail);
    }

    bool remove(T& value) {
        juce::ScopedLock lock(mutex); // Thread-sicherer Zugriff
        if (isEmpty()) {
            return false;
        }

        value = buffer[tail];
        tail = (tail + 1) % maxSize;
        isFull = false;

        return true;
    }

    bool isEmpty() const {
        return (!isFull && (head == tail));
    }

    int size() const {
        if (isFull) {
            return maxSize;
        }

        if (head >= tail) {
            return head - tail;
        }
        else {
            return maxSize + head - tail;
        }
    }

private:
    T* buffer;               // C-Array für den Puffer
    size_t maxSize;           // Maximale Größe des Puffers
    size_t head;              // Kopfzeiger
    size_t tail;              // Schwanzzeiger
    bool isFull;              // Status, ob der Puffer voll ist
    juce::CriticalSection mutex;  // Synchronisation für Thread-Sicherheit
};
