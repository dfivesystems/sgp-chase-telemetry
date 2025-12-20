#include "EventDispatcher.h"
#include "../logging/Logger.h"

EventDispatcher::EventDispatcher() {
    Logger::instance().info("EventDispatcher", "Starting event dispatcher");
    threadPool = std::vector<std::thread>(4);
    for(size_t i = 0; i < threadPool.size(); i++){
        Logger::instance().trace("EventDispatcher", "Creating event thread " + std::to_string(i));
        threadPool[i] = std::thread(&EventDispatcher::asyncThreadHandler, this);
    }
}

EventDispatcher::~EventDispatcher() {
    std::unique_lock<std::mutex> lock(threadLock);
    quitting = true;
    notifier.notify_all();
    threadLock.unlock();
    for(size_t i = 0; i < threadPool.size(); i++) {
        if(threadPool[i].joinable()) {
            threadPool[i].join();
        }
    }
}

void EventDispatcher::subscribe(EventType eventType, EventListener *listener) {
    //TODO: Add event type to logging
    Logger::instance().debug("EventDispatcher", "Adding listener");
    std::cout << "Adding listener" << std::endl;
    listeners[eventType].emplace_back(listener);
}

void EventDispatcher::unsubscribe(EventType eventType, EventListener *listener) {
    //TODO: Add event type to logging
    Logger::instance().debug("EventDispatcher", "Removing listener");
    listeners[eventType].remove(listener);
}

void EventDispatcher::dispatchDirect(std::shared_ptr<Event> ev) {
    //TODO: Add event type to logging
    Logger::instance().trace("EventDispatcher", "Dispatching direct event");
    for(auto listener : listeners[ev->eventType()]){
        listener->notifyMessage(ev);
    }
}

void EventDispatcher::dispatchAsync(std::shared_ptr<Event> ev) {
    std::unique_lock<std::mutex> lock(threadLock);
    Logger::instance().trace("EventDispatcher", "Dispatching async event");
    for(auto listener : this->listeners[ev->eventType()]) {
    workerQueue.push([listener, ev] {
        listener->notifyMessage(ev);
    });}
    notifier.notify_one();
}

void EventDispatcher::asyncThreadHandler() {
    std::unique_lock<std::mutex> lock(threadLock);
    while(!quitting) {
        notifier.wait(lock, [this] { return (!workerQueue.empty() || quitting); });
        if (!quitting && !workerQueue.empty()) {
            auto job = std::move(workerQueue.front());
            workerQueue.pop();
            lock.unlock();
            job();
            lock.lock();
        }
    }
}
