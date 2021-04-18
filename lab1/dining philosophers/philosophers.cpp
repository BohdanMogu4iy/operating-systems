#include <iostream>

#include <thread>
#include <mutex>

#define PHILOSOPHERS 4



mutex outputMutex;

struct fork
{
    mutex mutex;
};

struct table
{
    array<fork, PHILOSOPHERS> forks;
};

class philosopher
{
private:
    int const id;
    fork& left_fork;
    fork& right_fork;
    thread lifethread;
public:

    philosopher(int name, fork & l, fork & r): id(name), left_fork(l), right_fork(r), lifethread(&philosopher::dine, this)
    {}

    ~philosopher(){
        lifethread.join();

    }

    void start(){

    }

    [[noreturn]] thread dine(){
        while (true){
            {
                lock_guard<mutex> out(outputMutex);
                cout << "philosopher :" << this->id << " tries to eat" << endl;
            }
            eat();
            {
                lock_guard<mutex> out(outputMutex);
                cout << "philosopher :" << this->id << " stops trying" << endl;
            }
        }
    }

    void eat(){
        left_fork.mutex.try_lock();
        {
            {
                lock_guard<mutex> out(outputMutex);
                cout << "philosopher :" << this->id << " took left fork" << endl;
            }
            right_fork.mutex.try_lock();
            {
                {
                    lock_guard<mutex> out(outputMutex);
                    cout << "philosopher :" << this->id << " took right fork, so he is able to eat))" << endl;
                }
                this_thread::sleep_for(chrono::seconds(5));
                right_fork.mutex.unlock();
            }
            left_fork.mutex.unlock();
        }
    }

};

void diner() {
    table table;
    cout << "Dinner started!" << endl;
    philosopher philosophers[PHILOSOPHERS] = {
            {0, table.forks[0], table.forks[1]},
            {1, table.forks[1], table.forks[2]},
            {2, table.forks[2], table.forks[3]},
            {3, table.forks[3], table.forks[0]}
    };

}

int main()
{
    diner();
    cout << "Dinner done!";
    return 0;
}