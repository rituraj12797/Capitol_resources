#include<iostream>
#include<thread>
#include<vector>
#include<fstream>

#include "imp_macros.h"
#include "lf_queue.h"
#include "thread_utils.h"
#include "time_util.h"
#include "logger.h"


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

/*
  // START 2 threads 

  // 1 will be Logger 1 will be writer and 1 lfqueue for them to comm;

  // PREPARE TEST SET 
  // int n = 100; // test set size
  // std::vector<Order> test_set(n);

  // for(int i = 0 ; i < n ; i++ ) {
  //   test_set[i].qntty = (i%98);
  //   test_set[i].buy = (i%2 == 0 ? true : false);
  //   test_set[i].stock_id = i;
  // }

  // // DEFINE THE COMM CHANNEL
  // internal_lib::LFQueue<Order> comm(n*100); // defined a LFQueue of 100M size.
  // std::string path = "../logs/demo.log";

  // // WRITE THREAD LAMBDAS

  // auto producer_lambda = [](internal_lib::LFQueue<Order>& comm, std::vector<Order>& test_set) noexcept {

  //   for(size_t i = 0; i < test_set.size(); i++) {
  //       auto write_pointer = comm.getNextWrite();

  //     while(write_pointer == nullptr) { // BACKPRESSURE
  //       std::this_thread::yield();
  //       write_pointer = comm.getNextWrite();
  //     }
  //     std::cout<<" producing \n";

  //     *write_pointer = test_set[i];
  //     comm.updateWrite();

  //   }

  // };


  // auto consumer_logger_lambda = [](internal_lib::LFQueue<Order>& comm, std::string& path) noexcept {
  //     // logger thread

  //     std::ofstream file(path,std::ios::app); // std::ios::app ----> append mode do nto over write only append 

  //     if(!file.is_open()) {
  //       std::cerr<<" ERROR : LOG file could not be accessed \n";
  //       return ;
  //     }

  //     auto read_pointer = comm.getNextRead();

  //     // IDEALLY WE WOULD NTO CLOSE THIS AS IN OUR SERVER THIS WOULD KEEP RUNNING, BUT FOR LEARNIG PURPOSE WE WILL RUN A LOOP NOT WITH TREU BUT WITH SOEM CONSTRINTS SO THAT THREADS MAY TERMINATE AND PROCESS ENDS 
  //     // IN OUR SCENARIO SERVER NEVER STOPS SO THREADS WILL RUN INDEFINITELY
  //     for(int i = 0; i< 100; i++) {
  //       // read only 1000 timnes 
  //       read_pointer = comm.getNextRead();
  //       while(read_pointer == nullptr ) {
  //         std::this_thread::yield();
  //         read_pointer = comm.getNextRead();
  //       }
  //       file << "LOG : "<<" Object_id : "<<read_pointer->stock_id<<" type : "<<(read_pointer->buy ? "buy" : "sell")<<" Qntty : "<<read_pointer->qntty<<"\n";
  //       std::cout<<"Reading \n";
  //       comm.updateRead();
  //    } 

      

  // };



  // // Define threads 
  // auto producer_thread = internal_lib::createAndStartThread(1," producer",producer_lambda,std::ref(comm),std::ref(test_set));
  // auto consumer_thread = internal_lib::createAndStartThread(2,"consumer",consumer_logger_lambda,std::ref(comm),std::ref(path));

  // if(consumer_thread != nullptr) {
  //   consumer_thread->join();
  // }

  // if(producer_thread != nullptr) {
  //   producer_thread->join();
  // }

  // std::cout<<"FINISHED\n";

*/


  
  // define the buffers 
  internal_lib::LFQueue<internal_lib::LogElement> lob_to_log(50);
  internal_lib::LFQueue<internal_lib::LogElement> ogw_to_log(50);
  internal_lib::LFQueue<internal_lib::LogElement> mkt_to_log(50); 

  // test set -> create dummy data
  std::vector<internal_lib::LogElement> LOB_LOGS;
  std::vector<internal_lib::LogElement> ORDER_GATEWAY_LOGS;
  std::vector<internal_lib::LogElement> MARKET_DATA_PUBLISHER_LOGS;

  LOB_LOGS.reserve(50);
  ORDER_GATEWAY_LOGS.reserve(50);
  MARKET_DATA_PUBLISHER_LOGS.reserve(50);

  std::cout << "Generating Test Data...\n";

  for(int i = 0; i < 50; i++) {
    internal_lib::LogElement xu, yu, zu; 

    xu.time_stamp = uint64_t(internal_lib::getCurrentNanos()); 
    yu.time_stamp = uint64_t(internal_lib::getCurrentNanos());
    zu.time_stamp = uint64_t(internal_lib::getCurrentNanos());

    xu.component = internal_lib::ComponentId::ORDER_GATEWAY;
    yu.component = internal_lib::ComponentId::LOB_ENGINE;
    zu.component = internal_lib::ComponentId::MKT_DATA;

    xu.core_id = int32_t(1);
    yu.core_id = int32_t(2);
    zu.core_id = int32_t(3);

    xu.string_token = int32_t(1); 
    yu.string_token = int32_t(2);
    zu.string_token = int32_t(3);

    xu.data_object.ogw.z = i;
    yu.data_object.lob.y = i * 2;
    zu.data_object.mkt.x = i * 5;

    ORDER_GATEWAY_LOGS.push_back(xu);
    LOB_LOGS.push_back(yu);
    MARKET_DATA_PUBLISHER_LOGS.push_back(zu);
  }

  
  std::string file_path = "../log/result.log"; // Simple path
  
  internal_lib::Async_Logger logger(file_path, &mkt_to_log, &lob_to_log, &ogw_to_log); 



  auto og_lambda = [](internal_lib::LFQueue<internal_lib::LogElement>& queue, std::vector<internal_lib::LogElement>& logs){   
     for(auto &a : logs) {
         auto write_ptr = queue.getNextWrite(); 

         while(write_ptr == nullptr) {
             std::this_thread::yield();
             write_ptr = queue.getNextWrite();
         }
         *write_ptr = a; 
         queue.updateWrite(); 
     }
     std::cout << "OGW Done\n";
  };

  auto lob_lambda = [](internal_lib::LFQueue<internal_lib::LogElement>& queue, std::vector<internal_lib::LogElement>& logs){   
    for(auto &a : logs) {
            auto write_ptr = queue.getNextWrite();
            while(write_ptr == nullptr) {
                std::this_thread::yield();
                write_ptr = queue.getNextWrite();
            }
            *write_ptr = a; 
            queue.updateWrite(); 
        }
    std::cout << "LOB Done\n";
  };

  auto mkt_lambda = [](internal_lib::LFQueue<internal_lib::LogElement>& queue, std::vector<internal_lib::LogElement>& logs){   
    for(auto &a : logs) {
            auto write_ptr = queue.getNextWrite();
            while(write_ptr == nullptr) {
                std::this_thread::yield();
                write_ptr = queue.getNextWrite();
            }
            *write_ptr = a; 
            queue.updateWrite(); 
        }
    std::cout << "MKT Done\n";
  };

  
  std::cout << "Starting Threads...\n";

  auto t1 = internal_lib::createAndStartThread(0, "logger", [&logger](){ logger.run(); });

  auto t2 = internal_lib::createAndStartThread(1, "OrderGateway", og_lambda, std::ref(ogw_to_log), std::ref(ORDER_GATEWAY_LOGS));
  
  auto t3 = internal_lib::createAndStartThread(2, "LOB", lob_lambda, std::ref(lob_to_log), std::ref(LOB_LOGS));

  auto t4 = internal_lib::createAndStartThread(3, "MKT", mkt_lambda, std::ref(mkt_to_log), std::ref(MARKET_DATA_PUBLISHER_LOGS));

  t2->join();
  t3->join();
  t4->join();

  


  std::cout << "Producers finished. Allowing logger to drain...\n";
  std::this_thread::sleep_for(std::chrono::seconds(2));

  // logger.stop(); 
  // t1->join();
  // first tell the logger thread to stop 
  logger.stop();

  // then stop the loggr thread 
  t1->join();

  delete t1; delete t2; delete t3; delete t4;

  std::cout << "Test Complete. Check result.log\n";
  return 0;





}

