#include <JuceHeader.h>
#include <atomic>
#include <vector>

template <typename T>
class ThreadSafeRingBuffer {
public:
    explicit ThreadSafeRingBuffer(size_t size)
        : buffer(size), maxSize(size), head(0), tail(0), isFull(false) {}

    // Add a new value to the ring buffer
    void add(const T& value) {
        juce::ScopedLock lock(mutex);

        buffer[head] = value;
        head = (head + 1) % maxSize;

        if (head == tail) {  // Puffer ist voll
            tail = (tail + 1) % maxSize;
            isFull = true;
        }
        else {
            isFull = false;
        }
    }

    // Remove an item from the ring buffer
    bool remove(T& value) {
        juce::ScopedLock lock(mutex);
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
        juce::ScopedLock lock(mutex);
        return (!isFull && (head == tail));
    }

private:
    std::vector<T> buffer;  // Vector statt C-Array
    const size_t maxSize;
    std::atomic<size_t> head;  // Atomare Kopfzeiger
    std::atomic<size_t> tail;  // Atomare Schwanzzeiger
    std::atomic<bool> isFull;  // Atomarer Status
    mutable juce::CriticalSection mutex;
};
