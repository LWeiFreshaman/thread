#ifndef THREAD_POOL__
#define THREAD_POOL__

#include <thread>
#include <iostream>
#include <future>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>

using namespace std;

class ThreadPool
{
public:
    ThreadPool(size_t num = 1);
    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F f, Args... args)
    ->future<typename result_of<F(Args...)>::type>;

private:
    size_t threadNum;
    std::vector<thread> pool;
    queue<function<void(void)>> tasks;
    mutex m;
    condition_variable cv;
    bool stop;
};

ThreadPool::ThreadPool(size_t num) : threadNum(num), stop(false)
{
    for (int i = 0; i < threadNum; ++i)
    {
        pool.emplace_back(
            thread(
            [this](){
                while (true)
                {
                    function<void(void)> task;
                    {
                        unique_lock<mutex> ul(m);
                        cv.wait(ul, 
                            [this](){return stop || !tasks.empty();});
                        if (stop && tasks.empty())
                            return;
                        if (!tasks.empty())                         //非空
                        {   
                            task = std::move(tasks.front());        //从队列中取出任务执行        
                            tasks.pop();
                        } 
                    }
                    task();                                         //执行
                }
            })
        );
    }
}

ThreadPool::~ThreadPool()
{
    {
        unique_lock<mutex> ul(m);
        stop = true;
    }

    cv.notify_all();
    for (auto &iter : pool)
    {
        iter.join();                                                //等待所有线程结束
    }
}

template<class F, class... Args>
    auto ThreadPool::enqueue(F f, Args... args)
    ->future<typename result_of<F(Args...)>::type>
    {
        using return_type = typename result_of<F(Args...)>::type;   //取返回值类型

        auto task = make_shared<packaged_task<return_type()>>(      //这里为啥用shared_ptr
            bind(forward<F>(f), forward<Args>(args)...)             //怎么改成lambda
        );

        auto fut = task->get_future();                              //在这里存储结果

        {
            unique_lock<mutex> ul(m);
            if (stop)
                throw runtime_error("The thread pool has stopped.");
            
            tasks.emplace([task](){(*task)();});                     //暂时没看懂
        }
        cv.notify_one();

        return fut;
    }

#endif