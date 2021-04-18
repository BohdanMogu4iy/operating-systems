#include <iostream>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

#define MAX_SIZE 10
#define WORKERS 3
#define CONSUMERS 2

using namespace std;

mutex workerMutex;
mutex consumerMutex;
mutex notEmptyMutex;
mutex notFullMutex;
mutex listMutex;
mutex listSizeMutex;
mutex outputMutex;
condition_variable notEmpty, notFull;

typedef struct Node{
    int product_value;
    Node* next;
    Node* prev;
}PRODUCT, *PPRODUCT;


class list {
private:
    int max_length;
    int length;
    int productCounter;
    PPRODUCT head;
    PPRODUCT tail;
public:

    list(int maxLength) {
        this->head = nullptr;
        this->tail = nullptr;
        length = 0;
        productCounter = 0;
        max_length = maxLength;
    }

    int getMaxLength(){
        lock_guard<mutex> gl(listSizeMutex);
        return this->max_length;
    }

    int getLength() {
        lock_guard<mutex> gl(listSizeMutex);
        return this->length;
    }

    bool isEmpty() {
        lock_guard<mutex> gl(listSizeMutex);
        return this->length==0;
    }

    bool isFull(){
        lock_guard<mutex> gl(listSizeMutex);
        return this->length == max_length;
    }

    void pushFront() {
        PPRODUCT item = new PRODUCT;
        item->product_value = ++productCounter;
        item->prev = nullptr;
        item->next = head;
        if(head == nullptr && tail == nullptr){
            tail = item;
        }
        else{
            head->prev = item;
        }
        head = item;
        {
            lock_guard<mutex> gl(listSizeMutex);
            ++length;
        }
    }

    void popBack() {
        PPRODUCT pop = tail;
        if(tail==head){
            tail = head = nullptr;
        }
        else{
            tail = pop->prev;
            tail->next = nullptr;
        }
        {
            lock_guard<mutex> gl(listSizeMutex);
            --length;
        }
        delete pop;
    }

    void printList(){
        PPRODUCT item = head;
        while (item != nullptr)
        {
            cout << item->product_value << " ";
            item = item->next;
        }
        cout << endl;
    }

};

list* storage;

[[noreturn]] void consumer(int id){
    while (true){
        {
            lock_guard<mutex> out(outputMutex);
            cout<< "Consumer :"<< id<< " starts" << endl;
        }
        consumerMutex.lock();
        listMutex.lock();
        if (storage->isEmpty()){
            {
                lock_guard<mutex> out(outputMutex);
                cout<< "Storage is EMPTY, so consumer: " << id <<" must wait" << endl;
            }
            listMutex.unlock();
            unique_lock<mutex> sleep(notEmptyMutex);
            notEmpty.wait(sleep, []{return storage->getLength() == 1;});
            listMutex.lock();
        }
        if(storage->getLength() == 1) {
            {
                lock_guard<mutex> out(outputMutex);
                cout<< "Consumer :"<< id<< " work with len(storage) <=1" << endl;
            }
            storage->popBack();
            {
                lock_guard<mutex> out(outputMutex);
                cout << "Consumer :"<< id << " popped!" << endl;
                storage->printList();
            }
            listMutex.unlock();
        } else{
            {
                lock_guard<mutex> out(outputMutex);
                cout<< "Consumer :"<< id<< " work with len(storage) > 1" << endl;
            }
            listMutex.unlock();
            storage->popBack();
            {
                lock_guard<mutex> out(outputMutex);
                cout << "Consumer :"<< id << " popped!" << endl;
                storage->printList();
            }
        }
        if(not storage->isFull()){
            notFull.notify_one();
        }
        consumerMutex.unlock();
        this_thread::sleep_for(chrono::seconds(5));
    }
}

[[noreturn]] void worker(int id){
    while (true){
        {
            lock_guard<mutex> out(outputMutex);
            cout<< "Worker :"<< id<< " starts" << endl;
        }
        workerMutex.lock();
        listMutex.lock();
        if (storage->isFull()){
            {
                lock_guard<mutex> out(outputMutex);
                cout<< "Storage is FULL, so worker: "<< id << " can relax" << endl;
            }
            listMutex.unlock();
            unique_lock<mutex> sleep(notFullMutex);
            notFull.wait(sleep, []{return storage->getLength() == storage->getMaxLength() - 1;});
            listMutex.lock();
        }
        if(storage->getLength() <= 1) {
            {
                lock_guard<mutex> out(outputMutex);
                cout<< "Worker :"<< id<< " work with len(storage) <=1" << endl;
            }
            storage->pushFront();
            {
                lock_guard<mutex> out(outputMutex);
                cout << "Worker :"<< id << " pushed!" << endl;
                storage->printList();
            }
            listMutex.unlock();
        } else{
            {
                lock_guard<mutex> out(outputMutex);
                cout<< "Worker :"<< id<< " work with len(storage) > 1" << endl;
            }
            listMutex.unlock();
            storage->pushFront();
            {
                lock_guard<mutex> out(outputMutex);
                cout << "Worker :"<< id << " pushed!" << endl;
                storage->printList();
            }
        }
        if(not storage->isEmpty()){
            notEmpty.notify_one();
        }
        workerMutex.unlock();
        this_thread::sleep_for(chrono::seconds(2));
    }
}

int main()
{
    storage = new list(MAX_SIZE);

    vector<thread> workers;
    vector<thread> consumers;

    for (int i = 0; i < WORKERS; ++i){
        workers.push_back(move(thread(worker, i+1)));
    }

    for (int i = 0; i < CONSUMERS; ++i){
        consumers.push_back(move(thread(consumer, i+1)));
    }

    for (auto &oneWorker : workers) {
        oneWorker.join();
    }

    for (auto &oneConsumer : consumers) {
        oneConsumer.join();
    }




    return 0;
}