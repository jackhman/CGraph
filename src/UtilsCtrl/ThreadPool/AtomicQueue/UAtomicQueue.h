/***************************
@Author: Chunel
@Contact: chunel@foxmail.com
@File: UAtomicQueue.h
@Time: 2021/7/2 11:28 下午
@Desc: 
***************************/

#ifndef CGRAPH_UATOMICQUEUE_H
#define CGRAPH_UATOMICQUEUE_H

#include <memory>
#include <mutex>
#include <queue>
#include <condition_variable>

#include "../UThreadPoolDefine.h"
#include "../UThreadObject.h"


template<typename T>
class UAtomicQueue : public UThreadObject {
public:
    UAtomicQueue() {}

    void waitPop(T& value) {
        CGRAPH_LOCK_GUARD lk(mutex_);
        cv_.wait(lk, [this] { return !queue_.empty(); });
        value = std::move(*queue_.front());
        queue_.pop();
    }

    bool tryPop(T& value) {
        CGRAPH_LOCK_GUARD lk(mutex_);
        if (queue_.empty()) {
            return false;
        }
        value = std::move(*queue_.front());
        queue_.pop();
        return true;
    }

    std::unique_ptr<T> waitPop() {
        CGRAPH_LOCK_GUARD lk(mutex_);
        cv_.wait(lk, [this] { return !queue_.empty(); });
        std::unique_ptr<T> result = queue_.front();
        queue_.pop();
        return result;
    }

    std::unique_ptr<T> tryPop() {
        CGRAPH_LOCK_GUARD lk(mutex_);
        if (queue_.empty()) {
            return std::unique_ptr<T>();
        }
        std::unique_ptr<T> res = queue_.front();
        queue_.pop();
        return res;
    }

    void push(T&& value) {
        std::unique_ptr<T> task(std::make_unique<T>(std::move(value)));
        CGRAPH_LOCK_GUARD lk(mutex_);
        queue_.push(std::move(task));
        cv_.notify_one();
    }

    bool empty() const {
        CGRAPH_LOCK_GUARD lk(mutex_);
        return queue_.empty();
    }

private:
    std::mutex mutex_;
    std::queue<std::unique_ptr<T>> queue_;
    std::condition_variable cv_;

};


#endif //CGRAPH_UATOMICQUEUE_H