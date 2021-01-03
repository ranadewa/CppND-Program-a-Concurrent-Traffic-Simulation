#include <iostream>
#include <random>
#include <thread>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 

    std::unique_lock<std::mutex> lock(_mtx);

    // this pointer here captures the current object by reference
    _cv.wait(lock, [this](){ return !_queue.empty();});
    
    auto msg = std::move(_queue.front());
    _queue.pop_front();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    std::unique_lock<std::mutex> lock(_mtx);
    _queue.emplace_back(msg);
    _cv.notify_one();
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    setCycleDuration();
}


void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.

    while (true)
    {
        auto phase = _messageQueue.receive();  // recieve is a blocking operation

        if(phase == TrafficLightPhase::green)
            return;
    }
    

}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread
    // when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

void TrafficLight::setCycleDuration()
{
    std::random_device rd;
    std::mt19937 engine(rd());
    std::uniform_int_distribution<int> dist(4, 6);

    _cycleDuration = dist(engine);
}

size_t TrafficLight::getDifferenceToCurrentTime(std::chrono::time_point<std::chrono::system_clock> lastUpdate)
{

    std::chrono::time_point<std::chrono::system_clock> current = std::chrono::system_clock::now();
    auto timeGap = std::chrono::duration_cast<std::chrono::seconds>(current - lastUpdate).count();

    return timeGap;
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    std::chrono::time_point<std::chrono::system_clock> lastUpdate = std::chrono::system_clock::now();

    while (true)
    {        
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        
        if(getDifferenceToCurrentTime(lastUpdate) >= _cycleDuration)
        {
            lastUpdate = std::chrono::system_clock::now();
            setCycleDuration();

            auto newPhase = (_currentPhase == TrafficLightPhase::red) ? TrafficLightPhase::green : TrafficLightPhase::red;
            _currentPhase = newPhase;
            _messageQueue.send(std::move(newPhase));
        }
    }
    
}

