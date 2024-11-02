/*
  ==============================================================================

    ThreadSafeRingBuffer.h
    Created: 22 Oct 2024 10:28:42am
    Author:  andre

  ==============================================================================
*/
#include <JuceHeader.h>

template <typename T, int BufferSize>
class ThreadSafeRingBuffer {
public:
    ThreadSafeRingBuffer() {}

    // Füge ein neues Element zum Ringpuffer hinzu
    void add(const T& value) {
        const int currentHead = head.get();
        buffer[currentHead] = value;

        head = (currentHead + 1) % BufferSize;

        // Wenn Kopf den Schwanz überschreibt, verschiebt sich der Schwanz
        if (head.get() == tail.get()) {
            tail = (tail.get() + 1) % BufferSize;
            isFull = true;
        }
        else {
            isFull = false;
        }
    }

    // Entferne ein Element aus dem Ringpuffer
    bool remove(T& value) {
        if (isEmpty()) {
            return false;
        }

        const int currentTail = tail.get();
        value = buffer[currentTail];

        tail = (currentTail + 1) % BufferSize;
        isFull = false;

        return true;
    }

    // Überprüfe, ob der Buffer leer ist
    bool isEmpty() const {
        return (!isFull.get() && (head.get() == tail.get()));
    }

private:
    T buffer[BufferSize];                     // Statisches Array für den Puffer
    juce::Atomic<int> head{ 0 };              // Atomarer Kopfzeiger
    juce::Atomic<int> tail{ 0 };              // Atomarer Schwanzzeiger
    juce::Atomic<bool> isFull{ false };       // Atomarer Status, ob der Puffer voll ist
};
