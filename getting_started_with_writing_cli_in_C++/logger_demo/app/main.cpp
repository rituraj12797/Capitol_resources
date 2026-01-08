#include<iostream>
#include<thread>
#include<vector>
#include<fstream>

#include "imp_macros.h"
#include "lf_queue.h"
#include "thread_utils.h"
#include "time_util.h"


struct Order {

  int qntty;
  bool buy;
  int stock_id;

  Order(int qt, bool bu, int id) {
    stock_id = id;
    buy = bu;
    qntty = qt ;
  }

  Order() {
    stock_id = 0;
    buy = true;
    qntty = 0;
  }
};


/*
************************** THIS IS JUST FOR DEMONSTRATING HOW LOGGER WORKS FUNCTIONALITY WISE **********************************************

WE WILL STUDY IN DEPTH HOW ACTUAL IMPLEMENTATION OF LOW LATENCY LOGGER WILL BE DONE PROPERLY

*/




int main() {

  // START 2 threads 

  // 1 will be Logger 1 will be writer and 1 lfqueue for them to comm;

  // PREPARE TEST SET 
  int n = 100; // test set size
  std::vector<Order> test_set(n);

  for(int i = 0 ; i < n ; i++ ) {
    test_set[i].qntty = (i%98);
    test_set[i].buy = (i%2 == 0 ? true : false);
    test_set[i].stock_id = i;
  }

  // DEFINE THE COMM CHANNEL
  internal_lib::LFQueue<Order> comm(n*100); // defined a LFQueue of 100M size.
  std::string path = "../logs/demo.log";

  // WRITE THREAD LAMBDAS

  auto producer_lambda = [](internal_lib::LFQueue<Order>& comm, std::vector<Order>& test_set) noexcept {

    for(size_t i = 0; i < test_set.size(); i++) {
        auto write_pointer = comm.getNextWrite();

      while(write_pointer == nullptr) { // BACKPRESSURE
        std::this_thread::yield();
        write_pointer = comm.getNextWrite();
      }
      std::cout<<" producing \n";

      *write_pointer = test_set[i];
      comm.updateWrite();

    }

  };


  auto consumer_logger_lambda = [](internal_lib::LFQueue<Order>& comm, std::string& path) noexcept {
      // logger thread

      std::ofstream file(path,std::ios::app); // std::ios::app ----> append mode do nto over write only append 

      if(!file.is_open()) {
        std::cerr<<" ERROR : LOG file could not be accessed \n";
        return ;
      }

      auto read_pointer = comm.getNextRead();

      // IDEALLY WE WOULD NTO CLOSE THIS AS IN OUR SERVER THIS WOULD KEEP RUNNING, BUT FOR LEARNIG PURPOSE WE WILL RUN A LOOP NOT WITH TREU BUT WITH SOEM CONSTRINTS SO THAT THREADS MAY TERMINATE AND PROCESS ENDS 
      // IN OUR SCENARIO SERVER NEVER STOPS SO THREADS WILL RUN INDEFINITELY
      for(int i = 0; i< 100; i++) {
        // read only 1000 timnes 
        read_pointer = comm.getNextRead();
        while(read_pointer == nullptr ) {
          std::this_thread::yield();
          read_pointer = comm.getNextRead();
        }
        file << "LOG : "<<" Object_id : "<<read_pointer->stock_id<<" type : "<<(read_pointer->buy ? "buy" : "sell")<<" Qntty : "<<read_pointer->qntty<<"\n";
        std::cout<<"Reading \n";
        comm.updateRead();
     } 

      

  };



  // Define threads 
  auto producer_thread = internal_lib::createAndStartThread(1," producer",producer_lambda,std::ref(comm),std::ref(test_set));
  auto consumer_thread = internal_lib::createAndStartThread(2,"consumer",consumer_logger_lambda,std::ref(comm),std::ref(path));

  if(consumer_thread != nullptr) {
    consumer_thread->join();
  }

  if(producer_thread != nullptr) {
    producer_thread->join();
  }

  std::cout<<"FINISHED\n";
  return 0;

}

