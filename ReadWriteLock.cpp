#include <atomic>
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

class ReadWriteLock
{
public:
    ReadWriteLock() 
        : writeCount(0), readCount(0) { }
    
    void read_lock();
    void read_unlock();
    void write_lock();
    void write_unlock();

private:
    int writeCount;
    int readCount;
    mutex mt;
    condition_variable cv;
};

void ReadWriteLock::read_lock()
{
    unique_lock<mutex> loc(mt);
    cv.wait(loc, [this] { return writeCount == 0;});
    ++readCount;
    cout << "read lock." << endl;
}

void ReadWriteLock::read_unlock()
{
    unique_lock<mutex> loc(mt);
    cout << "read unlock." << endl;
    --readCount;
    cv.notify_all();
}

void ReadWriteLock::write_lock()
{
    unique_lock<mutex> loc(mt);
    cv.wait(loc, [this] { return writeCount == 0;});
    ++writeCount;
    cv.wait(loc, [this] { return readCount == 0;});
    cout << "write lock." << endl;
}

void ReadWriteLock::write_unlock()
{
    unique_lock<mutex> loc(mt);
    --writeCount;
    cout << "write unlock." << endl;
    cv.notify_all();
}

int main()
{
    ReadWriteLock rwl;
    thread t1([] (ReadWriteLock &rwl) {
        rwl.read_lock();
        cout << "thread1" << endl;
        this_thread::sleep_for(5000ms);
        rwl.read_unlock();
    } ,std::ref(rwl));

    thread t2([] (ReadWriteLock &rwl) {
        rwl.read_lock();
        cout << "thread2" << endl;
        this_thread::sleep_for(5000ms);
        rwl.read_unlock();
    } ,std::ref(rwl));

    thread t3([] (ReadWriteLock &rwl) {
        rwl.write_lock();
        cout << "thread3" << endl;
        this_thread::sleep_for(5000ms);
        rwl.write_unlock();
    }, std::ref(rwl));

    thread t4([] (ReadWriteLock &rwl) {
        rwl.read_lock();
        cout << "thread4" << endl;
        this_thread::sleep_for(5000ms);
        rwl.read_unlock();
    } ,std::ref(rwl));

    thread t5([] (ReadWriteLock &rwl) {
        rwl.read_lock();
        cout << "thread5" << endl;
        this_thread::sleep_for(5000ms);
        rwl.read_unlock();
    } ,std::ref(rwl));

    thread t6([] (ReadWriteLock &rwl) {
        rwl.read_lock();
        cout << "thread6" << endl;
        this_thread::sleep_for(5000ms);
        rwl.read_unlock();
    } ,std::ref(rwl));

    thread t7([] (ReadWriteLock &rwl) {
        rwl.write_lock();
        cout << "thread7" << endl;
        this_thread::sleep_for(5000ms);
        rwl.write_unlock();
    }, std::ref(rwl));
    
    thread t8([] (ReadWriteLock &rwl) {
        rwl.write_lock();
        cout << "thread8" << endl;
        this_thread::sleep_for(5000ms);
        rwl.write_unlock();
    }, std::ref(rwl));

    t3.join();
    t1.join();
    t2.join();
    t4.join();
    t7.join();
    t8.join();
    t5.join();   
    t6.join();
}