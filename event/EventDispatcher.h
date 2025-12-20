#ifndef EVENTDISPATCHER_H
#define EVENTDISPATCHER_H
#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <list>
#include <map>

#include "Event.h"

class EventListener{
public:
    virtual void notifyMessage(std::shared_ptr<Event> ev) = 0;
};

class EventDispatcher {
    typedef std::function<void(void)> fp_t;
public:
    static EventDispatcher& instance(){
        static EventDispatcher i;
        return i;
    }
    ~EventDispatcher();
    void dispatchDirect(std::shared_ptr<Event> ev);
    void dispatchAsync(std::shared_ptr<Event> ev);
    void subscribe(EventType eventType, EventListener* listener);
    void unsubscribe(EventType eventType, EventListener* listener);

    EventDispatcher(const EventDispatcher& other) = delete;
    EventDispatcher& operator= (const EventDispatcher &other) = delete;
    EventDispatcher(const EventDispatcher&& other) = delete;
    EventDispatcher& operator= (const EventDispatcher&& other) = delete;

private:
    std::mutex threadLock;
    std::vector<std::thread> threadPool;
    std::queue<fp_t> workerQueue;
    std::map<EventType, std::list<EventListener*>> listeners;
    std::condition_variable notifier;

    bool quitting = false;
    void asyncThreadHandler();

    EventDispatcher();
};



#endif //EVENTDISPATCHER_H
