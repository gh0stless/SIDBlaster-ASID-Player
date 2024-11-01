#include <JuceHeader.h>
#include <vector>

template <typename T>
class ThreadSafeRingBuffer {
public:
    explicit ThreadSafeRingBuffer(int size)
        : buffer(size), maxSize(size), head(0), tail(0), isFull(false) {}

    // Add a new value to the ring buffer
    void add(const T& value) {
        juce::ScopedLock lock(mutex);

        buffer[head.get()] = value;
        head = (head.get() + 1) % maxSize;

        if (head.get() == tail.get()) {  // Buffer ist voll
            tail = (tail.get() + 1) % maxSize;
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

        value = buffer[tail.get()];
        tail = (tail.get() + 1) % maxSize;
        isFull = false;

        return true;
    }

    // Check if the buffer is empty
    bool isEmpty() const {
        return (!isFull.get() && (head.get() == tail.get()));
    }

private:
    std::vector<T> buffer;              // Vector statt C-Array
    const int maxSize;
    juce::Atomic<int> head{ 0 };          // Atomare Kopfzeiger
    juce::Atomic<int> tail{ 0 };          // Atomare Schwanzzeiger
    juce::Atomic<bool> isFull{ false };   // Atomarer Status
    mutable juce::CriticalSection mutex;
};
